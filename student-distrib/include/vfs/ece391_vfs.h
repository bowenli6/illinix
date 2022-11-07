#ifndef _ECE391_VFS_H
#define _ECE391_VFS_H

#define OPEN_MAX    8               /* Each task can have up to 8 open files. */
#define stdin       0               /* Standard input from the terminal. */
#define stdout      1               /* Standard output to the terminal. */

#include <vfs/file.h>

int32_t fd_init();
int32_t file_open(const int8_t *fname);
int32_t file_close(int32_t fd);
int32_t file_read(int32_t fd, void *buf, int32_t nbytes);
int32_t file_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t directory_open(const int8_t *fname);
int32_t directory_close(int32_t fd);
int32_t directory_read(int32_t fd, void *buf, int32_t nbytes);
int32_t directory_write(int32_t fd, const void *buf, int32_t nbytes);


/* Interactions between a process and a filesystem */
typedef struct {
    uint32_t count;     /* Number of processes sharing this table */
    uint32_t umask;     /* Bit mask used when opening the file to set the file permissions */
    dentry_t *root;     /* Dentry of the root directory */
    dentry_t *pwd;      /* Dentry of the current working directory */
} vfs_t;



typedef struct {
    uint32_t count;         /* Number of processes sharing this table */
    uint32_t max_fd;        /* Current maximun number of file objects */
    file_t fd[OPEN_MAX];    /* Pointers to array of file object pointers */
} files;




#endif /* _ECE391_VFS_H */
