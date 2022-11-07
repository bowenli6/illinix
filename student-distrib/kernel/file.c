#include <vfs/file.h>
#include <vfs/ece391_vfs.h>
#include <lib.h>
#include <pro/process.h>
#include <io.h>

/**
 * @brief Initialize the file object.
 * 
 * @param fd : A starting file descriptor. 
 * @param file : A file object that to be set.
 * @param dentry : A descriptor entry taht will be included in the file object.
 * @param op : A file operation list that will be included in the file object.
 * @return int32_t : A file descriptor on success, -1 on failure.
 */
int32_t file_init(int32_t fd, file_t *file, dentry_t *dentry, file_op *op) {
    int i;

    for (i = fd; i < OPEN_MAX; ++i) {
        /* If there is an unused file object. */
        if (CURRENT->fds.fd[fd].f_flags == UNUSED) {
            /* Copy data to the new file object. */
            memcpy((void*)(&(file->f_dentry)), (void*)dentry, sizeof(dentry_t));
            file->f_op = *op;
            file->f_count = 1;
            file->f_flags = INUSED;
            file->f_pos = 0;
            return i;   /* Return the file descriptor. */
        }
    }
    puts("ERROR: The file descriptors limit has been reached.\n");
    return -1;
}
