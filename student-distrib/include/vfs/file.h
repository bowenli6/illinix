#ifndef _FILE_H
#define _FILE_H

#include <types.h>
#include <drivers/fs.h>

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
    uint32_t f_pos;         /* Current file offset (file pointer). */
} file_t;


#endif /* _FILE_H */
