#include <vfs/vfs.h>


/**
 * @brief Initialize the virtual file system.
 * 
 */
void vfs_init() {
    
}


/**
 * @brief system call for opening a file.
 * 
 * @return int : The file descriptor of the opened file.
 */
int sys_open() {
    // TODO!

    /* Find an empty slot in the vfs' file objects array. */
    // int fd = get_unused_fd();
    return 0;
}

/**
 * @brief 
 * 
 * @return int 
 */
int sys_read() {
    return 0;
}


/**
 * @brief 
 * 
 * @return int 
 */
int sys_write() {
    return 0;
}


/**
 * @brief 
 * 
 * @return int 
 */
int sys_close() {
    return 0;
}
