#include <vfs/file.h>
#include <vfs/ece391_vfs.h>
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
        if (vfs.fd[fd]->f_flags == UNUSED) {
            file->f_dentry = dentry;
            file->f_op = op;
            file->f_count = 1;
            file->f_flags = INUSED;
            file->f_pos = 0;
            return i;
        }
    }
    puts("ERROR: The file descriptors limit has been reached.\n");
    return -1;
}
