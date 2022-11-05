#ifndef _FILE_H
#define _FILE_H

#include <types.h>
#include <drivers/fs.h>
// #include <pro/process.h>

#define INUSED      1                      /* A file is inused. */
#define UNUSED      0                      /* A file is unused. */
#define MAXCOUNT    ((1 << 31) - 1)        /* Maximum number of bytes that can be read or written with a single operation. */

typedef struct {
    int32_t (*open)(const int8_t *);
    int32_t (*close)(int32_t);
    int32_t (*read)(int32_t, void *, int32_t);
    int32_t (*write)(int32_t, const void *, int32_t);
} file_op;


/* A file stores information about the interaction 
 * between an open file and a process. The information
 * exists only in kernel memory during the period when
 * a process has the file open. */
typedef struct {
    dentry_t f_dentry;      /* The dentry for this file. */
    file_op  f_op;          /* Pointer to the file operation table. */
    uint32_t f_count;       /* File object's reference count. */
    uint32_t f_flags;       /* Flags specified when opening the file. */
    uint32_t f_pos;         /* Current file offset (file pointer). */
    uint32_t f_maxcount;    /* Maximum number of bytes that can be read or written with a single operation. */
    // mode_t   f_mode;        /* Process access mode. */
    // uid_t    f_uid;         /* Owner identifier. */
    // gid_t    f_gid;         /* Group identifier. */ 
} file_t;



int32_t file_init(int32_t fd, file_t *file, dentry_t *dentry, file_op *op);

#endif /* _FILE_H */
