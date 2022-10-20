#ifndef _FILE_H
#define _FILE_H

#include <types.h>
#include <vfs/inode.h>
#include <vfs/dentry.h>
#include <pro/process.h>

typedef struct {
} file_op;


/* A file stores information about the interaction 
 * between an open file and a process. The information
 * exists only in kernel memory during the period when
 * a process has the file open. */
typedef struct {
    uint32_t f_count;       /* File object's reference count. */
    file_op *f_op;          /* Pointer to the file operation table. */
    uint32_t f_flags;       /* Flags specified when opening the file. */
    mode_t   f_mode;        /* Process access mode. */
    uid_t    f_uid;         /* Owner identifier. */
    gid_t    f_gid;         /* Group identifier. */ 
} file_t;


typedef struct {
    int fd;
} fd_t;

#endif /* _FILE_H */
