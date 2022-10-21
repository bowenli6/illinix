#include <drivers/fs.h>
#include <lib.h>
#include <io.h>


fs_t fs;        /* Stores the file system. */

/**
 * @brief Initialize the file system.
 * 
 * @param start_addr : The start address of the file system in the memory.
 */
void fs_init(uint32_t start_addr) {
    void *addr = (void *)start_addr;

    fs.boot = (boot_block*)addr;                /* Load the boot block. */
    addr += BLOCK_SIZE;                         /* Get the address of the first inode block ('.'). */
    fs.inodes = (inode_t *)addr;                /* Load the inodes blocks. */
    addr += fs.boot->n_inode * BLOCK_SIZE;      /* Get the address of the first data block. */
    fs.data_block_addr = (data_block *)addr;    /* Load the data blocks. */
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
int32_t read_dentry_by_name(const uint8_t *fname, dentry_t *dentry) {
    if (!dentry) {
        puts("ERROR: dentry is undefined.\n");
        return -1;
    }
    int i;    
    for (i = 0; i < fs.boot->n_dir; ++i) {

        /* The current file name stored in the boot block. */
        int8_t *_fname = (int8_t *)(fs.boot->dirs[i].fname);
        if (!strncmp((int8_t *)fname, _fname, NAMESIZE)) {

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
        puts("ERROR: dentry is undefined.\n");
        return -1;
    }

    if ((index >= FILES_MAX) || (index >= fs.boot->n_dir)) {
        puts("ERROR: index is out of bounds.\n");
        return -1;
    }

    /* Copy dentry from boot block to the given dentry pointer. */
    *dentry = fs.boot->dirs[index];
    
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
    int i;
    int phy_pos;                    /* The physical position of the file to read. */
    virtual_pos vir_pos;            /* The virtual position of the file to read*/
    int nread_needed;               /* Number of bytes need to read. */
    int nread_each;                 /* Number of bytes read each time. */
    int nb_left;                    /* Number of bytes left in the data block. */
    inode_t file;                   /* The file inode. */
    int8_t *data_ptr;              /* The actuall data address to read within a data block. */

    if (inode >= fs.boot->n_inode) {
        puts("ERROR: invalid inode, no such inode exists in the file system.\n");
        return -1;
    }

    if (!buf) {
        puts("ERROR: buf is undefined.\n");
        return -1;
    }
    
    file = fs.inodes[inode];            /* Get the file inode. */

    if (fs.boot->dirs[inode].type < 2)  /* RTC or DIRECTORY */
        return 0;
        
    if (offset >= file.size) {
        puts("ERROR: offset is out of bounds. ");
        return -1;
    }

    nread_needed = length;
    nb_left = file.size;

    /* The index of bytes start to read. */
    phy_pos = offset; 

    /* The number of data blocks start to read. */
    vir_pos.nblock = phy_pos / BLOCK_SIZE;      

    /* The index of the starting data block we need to read. */
    vir_pos.iblock = file.data_block[vir_pos.nblock];

    /* The data block object of the starting data block we need to read. */
    vir_pos.datab = fs.data_block_addr[vir_pos.iblock];

    /* The index of the data we want to read. */
    vir_pos.idx = phy_pos % vir_pos.nblock;

    while (nread_needed > 0 && nb_left > 0) {
        nread_each = BLOCK_SIZE - vir_pos.idx; 
        data_ptr = (int32_t*)vir_pos.datab.data[vir_pos.idx];
        if (nread_each > nread_needed) {
            /* Numebr of bytes need to read is less than the remaining data size within the block. */
            if (nread_needed < nb_left) {
                memcpy((void*)buf, (void*)data_ptr, nread_needed);
                nread_needed = 0;
            } else {
                memcpy((void*)buf, (void*)data_ptr, nb_left);
                nread_needed -= nb_left;
            }
            break;
        } else {
            /* We need more blocks to read. */
            if (nread_needed < nb_left) {
                memcpy((void*)buf, (void*)data_ptr, nread_each);
                buf += nread_each;
                nb_left -= nread_each;
                nread_needed -= nread_each;

                /* Update vir_pos to the next data block used by the file inode. */
                vir_pos.iblock = file.data_block[++vir_pos.nblock];
                if (vir_pos.iblock >= fs.boot->n_datab) {
                    puts("ERROR: invalid data block found.\n");
                    return -1;
                }
                vir_pos.datab = fs.data_block_addr[vir_pos.iblock];
                vir_pos.idx = 0;    /* Start from beginning of the new data block. */
            } else {
                memcpy((void*)buf, (void*)data_ptr, nb_left);
                nread_needed -= nb_left;
                break;
            }
        }
    }
  
    return length - nread_needed;
}
