#include <drivers/fs.h>
#include <lib.h>


fs_t fs;        /* Stores the file system. */

/**
 * @brief Initialize the file system.
 * 
 * @param start_addr : The start address of the file system in the memory.
 */
void fs_init(uint32_t start_addr) {
    int i;

    void *addr = (void *)start_addr;
    fs.boot = (boot_block*)addr;        /* Load the boot block. */
    addr += BLOCK_SIZE;                 /* Get the address of the first inodes ('.'). */
    
    /* Update each inodes from addr. */
    for (i = 0; i < FILES_MAX; ++i) {
        fs.inodes[i] = *(inode_t *)addr;
        addr += BLOCK_SIZE;
    }

    /* Copy the starting addr of the data blocks from addr. */
    fs.data_block_addr = (data_block *)addr;
}


/**
 * @brief Fill in the @param dentry block with the file name, file
 * type, and inode number for the file.
 * 
 * @param fname : A filename.
 * @param dentry : A pointer to the directory entry structure.
 * @return int32_t : -1 on failure (non-existent file or invalid index),
 *                    0 on success.
 */
int32_t read_dentry_by_name(const uint8_t *fname, dentry_t *dentry) {
    if (!dentry) {
        puts("ERROR: dentry is undefined. \n");
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
 * @brief Fill in the @param dentry block with the file name, file
 * type, and inode number for the file.
 * 
 * @param index : The directory entry index.
 * @param dentry : A pointer to the directory entry structure.
 * @return int32_t : -1 on failure (non-existent file or invalid index), 
 *                    0 on success.
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry) {
    if (!dentry) {
        puts("ERROR: dentry is undefined. \n");
        return -1;
    }

    if ((index >= FILES_MAX) || (index >= fs.boot->n_dir)) {
        puts("ERROR: index is out of bounds. ");
        return -1;
    }

    /* Copy dentry from boot block to the given dentry pointer. */
    *dentry = fs.boot->dirs[index];
    
    return 0;
}


/**
 * @brief Reading up to @param length bytes starting from position @param offset
 * in the file with inode number @param inode. Place the bytes read in @param buf.
 * 
 * @param inode :
 * @param offset :
 * @param buf : 
 * @param length :
 * @return int32_t : -1 on failure (non-existent file or invalid inode number), 
 *                    number of bytes read on success.
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length) {

    return 0;
}


/**
 * @brief open the file named @param fname.
 * 
 * @param fname : The file name.
 * @return int : 0 on success, -1 on failure.
 */
int file_open(const uint8_t *fname) {
    // return read_dentry_by_name(fname)
    return 0;
}


int file_close() {
    return 0;
}

int file_read() {
    // read_data();
    return 0;
}


int file_write() {
    /* Not suppoted yet. */
    return -1;
}