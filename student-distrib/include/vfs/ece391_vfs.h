#ifndef _ECE391_VFS_H
#define _ECE391_VFS_H

#define OPEN_MAX    8               /* Each task can have up to 8 open files. */
#define stdin       0               /* Standard input from the terminal. */
#define stdout      1               /* Standard output to the terminal. */

#include <vfs/file.h>

int32_t vfs_init();
int32_t file_open(const int8_t *fname);
int32_t file_close(int32_t fd);
int32_t file_read(int32_t fd, void *buf, int32_t nbytes);
int32_t file_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t directory_open(const int8_t *fname);
int32_t directory_close(int32_t fd);
int32_t directory_read(int32_t fd, void *buf, int32_t nbytes);
int32_t directory_write(int32_t fd, const void *buf, int32_t nbytes);

typedef struct {
    file_t fd[OPEN_MAX];           /* Files that are currently opened. */
} ece391_vfs_t;

extern ece391_vfs_t vfs;




#endif /* _ECE391_VFS_H */
