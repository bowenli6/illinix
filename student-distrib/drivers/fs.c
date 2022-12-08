#include <drivers/fs.h>
#include <pro/process.h>
#include <access.h>
#include <kmalloc.h>
#include <errno.h>
#include <lib.h>
#include <boot/x86_desc.h>
#include <boot/page.h>

fs_t *fs;        /* Stores the file system. */


static int32_t validate_inode(uint32_t inode);
static int32_t validate_fname(const int8_t *fname);


/**
 * @brief Initialize the file system.
 * 
 * @param start_addr : The start address of the file system in the memory.
 */
void fs_init(uint32_t start_addr) {
    fs = kmalloc(sizeof(fs_t));
    boot_block *addr = (boot_block *)start_addr;
    fs->boot = addr++;                           /* Load the boot block. */
    fs->inodes = (inode_t *)addr;                /* Load the inodes blocks. */
    addr += fs->boot->n_inode;                   /* Get the address of the first data block. */
    fs->data_block_addr = (data_block *)addr;    /* Load the data blocks. */
}


/**
 * @brief Fill in the dentry block with the file name, file
 * type, and inode number for the file.
 * 
 * @param fname : A filename.
 * @param dentry : A pointer to the directory entry structure.
 * @return int32_t : -1 on failure (non-existent file or invalid index),
 *                    0 on success.
 */
int32_t read_dentry_by_name(const int8_t *fname, dentry_t *dentry) {
    if (!dentry) {
        return -1;
    }
    int i;    
    for (i = 0; i < fs->boot->n_dir; ++i) {

        /* The current file name stored in the boot block. */
        int8_t *_fname = (int8_t *)(fs->boot->dirs[i].fname);
        if (!strncmp(fname, _fname, NAMESIZE)) {

            /* Two file names are equal. */
            return read_dentry_by_index(i, dentry);
        }
    }
    return -1;  /* Not found, return an error. */
}


/**
 * @brief Fill in the dentry block with the file name, file
 * type, and inode number for the file.
 * 
 * @param index : The directory entry index.
 * @param dentry : A pointer to the directory entry structure.
 * @return int32_t : -1 on failure (non-existent file or invalid index), 
 *                    0 on success.
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry) {
    if (!dentry) {
        return -1;
    }

    if ((index >= FILES_MAX) || (index >= fs->boot->n_dir)) {
        return -1;
    }

    /* Copy dentry from boot block to the given dentry pointer. */
    *dentry = fs->boot->dirs[index];
    
    return 0;
}


/**
 * @brief Reading up to length bytes starting from position offset
 * in the file with inode number inode. Place the bytes read in buf.
 * 
 * @param inode : A inode number
 * @param offset : The offset of the file in bytes to read.
 * @param buf : A buffer array that copys the content from the file.
 * @param length : The number of bytes to read from the file.
 * @return int32_t : -1 on failure (non-existent file or invalid inode number), 
 *                    number of bytes read on success.
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length) {
    virtual_pos vir_pos;            /* The virtual position of the file to read*/
    int phy_pos;                    /* The physical position of the file to read. */
    int nread_needed;               /* Number of bytes need to read. */
    int nread_each;                 /* Number of bytes read each time. */
    int nb_left;                    /* Number of bytes left in the data block. */
    inode_t *file;                  /* The file inode. */
    int8_t *data_ptr;               /* The actuall data address to read within a data block. */
    int32_t errno;

    if ((errno = validate_inode(inode)) < 0) return errno;

    if (!buf) return -1;
    
    file = &fs->inodes[inode];            /* Get the file inode. */
        
    if (offset >= file->size) return 0;
    

    /* The index of bytes start to read. */
    phy_pos = offset; 

    /* The number of data blocks start to read. */
    vir_pos.nblock = phy_pos / BLOCK_SIZE;      

    /* The index of the starting data block we need to read. */
    vir_pos.iblock = file->data_block[vir_pos.nblock];

    /* The data block object of the starting data block we need to read. */
    vir_pos.datab = &fs->data_block_addr[vir_pos.iblock];

    /* The index of the data we want to read. */
    vir_pos.idx = phy_pos % BLOCK_SIZE;

    /* The number of bytes left in this file. */
    nb_left = file->size - (vir_pos.nblock * BLOCK_SIZE) - vir_pos.idx;

    if (length >= nb_left) {
        nread_needed = nb_left;
        length = nb_left;
    } else {
        nread_needed = length;
    }

    while (nread_needed) {
        nread_each = BLOCK_SIZE - vir_pos.idx; 
        data_ptr = &(vir_pos.datab->data[vir_pos.idx]);
        if (nread_each >= nread_needed) {
            /* Numebr of bytes need to read is less than the remaining data size within the block. */
            memcpy((void*)buf, (void*)data_ptr, nread_needed);
            nread_needed = 0;
            break;
        } else {
            /* We need more blocks to read. */
            memcpy((void*)buf, (void*)data_ptr, nread_each);
            buf += nread_each;
            nb_left -= nread_each;
            nread_needed -= nread_each;
            /* Update vir_pos to the next data block used by the file inode. */
            vir_pos.iblock = file->data_block[++vir_pos.nblock];
            if (vir_pos.iblock >= fs->boot->n_datab) {
                return -1;
            }
            vir_pos.datab = &fs->data_block_addr[vir_pos.iblock];
            vir_pos.idx = 0;    /* Start from beginning of the new data block. */
        }
    }
  
    return length - nread_needed;
}

/**
 * @brief Get the file size.
 * 
 * @param index The index of the dentry.
 * @return uint32_t : file size.
 */
uint32_t get_size(uint32_t index) {
    dentry_t dentry;
    read_dentry_by_index(index, &dentry);
    if (dentry.type < 2) 
        return 0;
    return fs->inodes[dentry.inode].size;
}


/**
 * @brief Load program image into user vitural address space
 * 
 * @param fname file name of the program
 * @param EIP: the address of the user program eip register
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
int32_t pro_loader(const int8_t *fname, uint32_t *EIP, thread_t* curr) {
    int i;
    int32_t errno;
    int32_t inode;
    inode_t file;
    uint8_t header[40];
    uint8_t eip_buf[4];
    uint8_t magic_number[4] = { 0x7f, 0x45, 0x4c, 0x46 };

    /* check if the file is a user-level executable file */
    if ((inode = validate_fname(fname)) < 0)
        return inode;
    
    /* get the file inode */
    file = fs->inodes[inode];            /* Get the file inode. */

    /* read header from the program image */
    if ((errno = read_data(inode, 0, header, 40)) < 0)
        return errno;

    /* check magic number */
    for (i = 0; i < 4; ++i) {
        if (header[i] != magic_number[i])
            return -1;
        eip_buf[i] = header[i + 24];
    }

    *EIP = *(uint32_t*)eip_buf;

    curr->vm.file_length = (file.size + PAGE_SIZE - 1) / PAGE_SIZE;
    vmalloc(curr->vm.map_list, curr->vm.file_length * PAGE_SIZE, PTE_RW | PTE_US);
    /* map the virtual memory space to to child */
    


    if ((errno = read_data(inode, 0, (uint8_t *)PROGRAM_IMG_BEGIN, file.size)) < 0) {
        return errno;
    }
    
    return 0;
}

/**
 * @brief Validate the input file name
 * 
 * @param inode : A file name of the file
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
static int32_t validate_fname(const int8_t *fname) {
    int32_t errno;
    dentry_t dentry;
    
    if ((errno = read_dentry_by_name(fname, &dentry)) < 0) 
        return errno;
    
    return dentry.inode;
}

/**
 * @brief Validate the input file inode
 * 
 * @param inode : inode of the file
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
static int32_t validate_inode(uint32_t inode) {
    if (inode >= fs->boot->n_inode)
        return -1;
    return 0;
}
