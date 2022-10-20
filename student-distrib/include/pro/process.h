#ifndef _PROCESS_H
#define _PROCESS_H

#include <types.h>
#include <vfs/vfs.h>

typedef uint32_t uid_t;

typedef uint32_t gid_t;

typedef struct {
    uid_t uid;
    gid_t gid;
    /* add more ...*/
} process_t;

// file_t *fgets(int fd);
// int fputs(file_t *file); 

#endif /* _PROCESS_H */
