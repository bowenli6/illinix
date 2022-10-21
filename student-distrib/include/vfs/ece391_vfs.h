#ifndef _ECE391_VFS_H
#define _ECE391_VFS_H

#define OPEN_MAX    8               /* Each task can have up to 8 open files. */
#define stdin       0               /* Standard input from the terminal. */
#define stdout      1               /* Standard output to the terminal. */

#include <drivers/fs.h>
#include <vfs/file.h>
#include <drivers/terminal.h>
#include <drivers/rtc.h>

int32_t vfs_init();
int32_t file_open(const int8_t *fname);
int32_t file_close(int32_t fd);
int32_t file_read(int32_t fd, void *buf, int32_t nbytes);
int32_t file_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t directory_open(const int8_t *fname);
int32_t directory_close(int32_t fd);
int32_t directory_read(int32_t fd, void *buf, int32_t nbytes);
int32_t directory_write(int32_t fd, const void *buf, int32_t nbytes);

/* File operation used for regular files. */
static file_op f_op = {
    .open = file_open,
    .close = file_close,
    .read = file_read,
    .write = file_write
};

/* Directory operation used for '.'. */
static file_op dir_op = {
    .open = directory_open,
    .close = directory_close,
    .read = directory_read,
    .write = directory_write
};

/* Terminal operation used for stdin and stdout. */
static file_op terminal_op = {
    .open = terminal_open,
    .close = terminal_close,
    .read = terminal_read,
    .write = terminal_write
};

/* RTC operation. */
// TODO


typedef struct {
    file_t *fd[OPEN_MAX];           /* Files that are currently opened. */
} ece391_vfs_t;

extern ece391_vfs_t vfs;




#endif /* _ECE391_VFS_H */
