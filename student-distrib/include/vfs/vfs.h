#ifndef _VFS_H
#define _VFS_H

#include <types.h>
#include <vfs/file.h>
#include <vfs/inode.h>
#include <vfs/dentry.h>

/* Flags for open() system call. */
#define O_RDONLY    0       /* Open for reading only. */
#define O_WRONLY    0       /* Open for writing only. */
#define O_RDWR      0       /* Open for both reading and writing. */
#define O_CREATE    0       /* Create the file if it does not exist. */
#define O_APPEND    0       /* Always write at end of the file. */


#define FD_CAPACITY     0   /* A process cannot use more than this many of file descriptors. */

typedef struct {

} vfsmount_t;

typedef struct {
    int mask;               /* Bit mask used when opening the file to set the file permissions. */
    uint32_t count;         /* The number of processes sharing this table. */
    dentry_t *root;         /* The dentry of the root directory. */
    dentry_t *pwd;          /* The dentry of the current working directory. */
    vfsmount_t *rootmnt;    /* Mounted filesystem object of the root directory. */
    vfsmount_t *pwdmnt;     /* Mounted filesystem object of the current working directory. */  
} vfs_t;



typedef struct {
    /* The index of the file_t pointer is the file descriptors. 
     * 0: stdin, 1: stdout, 2: stderr. */
    file_t **fd;            /* Pointer to array of file objects pointers. */
    fd_t   *open_fds;       /* Pointer to array of open file descriptors. */
    int    file_size;       /* Current maximum number of file objects. Init to 32. */
    int    fd_size;         /* Current maximum number of open file descriptors. */
} file_table;


typedef struct {
    inode_t inode;
    inode_t *prev;
    inode_t *next;
} i_list;

i_list unused;          /* The list of valid unused inodes. */
i_list inused;          /* The list of valid inused inodes. */
i_list idirty;          /* The list of dirty inodes. */


/* Systerm call interface provided by the VFS. */

int sys_open();



/* Not used yet. */
void mount();           /* Mount filesystems. */
void umount();          /* Unmount filesystems. */
void sys_fs();          /* Get filesystem information. */
void stat_fs();         /* Get filestem statistics. */
void chroot();          /* Change root directory. */
void chdir();           /* Manipulate directory entries. */


void dup();
void dup2();
void fcntl();



#endif /* _VFS_H */
