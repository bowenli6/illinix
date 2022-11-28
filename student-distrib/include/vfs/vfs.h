#ifndef _VFS_H_
#define _VFS_H_


#define OPEN_MAX    8               /* Each task can have up to 8 open files. */
#define stdin       0               /* Standard input from the terminal. */
#define stdout      1               /* Standard output to the terminal. */

#include <vfs/file.h>
#include <pro/process.h>

int32_t fd_init(thread_t *curr);
int32_t file_open(const int8_t *fname);
int32_t file_close(int32_t fd);
int32_t file_read(int32_t fd, void *buf, int32_t nbytes);
int32_t file_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t directory_open(const int8_t *fname);
int32_t directory_close(int32_t fd);
int32_t directory_read(int32_t fd, void *buf, int32_t nbytes);
int32_t directory_write(int32_t fd, const void *buf, int32_t nbytes);
// int32_t __open(int32_t fd, const int8_t *fname, file_type_t type, file_op *op);
int32_t do_open(const int8_t *filename);
int32_t do_close(int32_t fd);
int32_t do_read(int32_t fd, void *buf, uint32_t nbytes);
int32_t do_write(int32_t fd, const void *buf, uint32_t nbytes);


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


#endif /* _VFS_H_ */
