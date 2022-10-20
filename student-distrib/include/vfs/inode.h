#ifndef _INODE_H
#define _INODE_H

#include <types.h>
#include <vfs/file.h>
#include <vfs/superblock.h>
#include <pro/process.h>


typedef struct {

} mode_t;

typedef struct {

} inode_op;

/* A inode stores general information about a specific file. */
typedef struct {
    uint32_t i_ino;             /* The unique inode number. */
    uint32_t i_count;           /* The number of process using this inode. */
    mode_t   i_mode;            /* The File type and access rights. */
    uint32_t i_nlink;           /* The number of hard links. */
    uint32_t i_filesize;        /* File length in bytes. */
    uint32_t i_blocksize;       /* Block size in bytes. */
    uint32_t i_blocks;          /* The number of blocks of the file. */
    uint32_t i_bytes;           /* The number of bytes in the last block of the file. */
    inode_op *i_op;             /* The inode operations. */
    // file_op  *i_fop;            /* The Default file operations. */
    superblock_t *i_sb;         /* Pointer to the superblock object. */
    uint32_t i_state;           /* The inode state flags. */
    uint32_t i_flag;            /* The filesystem mount flag. */
    uid_t    i_uid;             /* Owner identifier. */
    gid_t    i_gid;             /* Group identifier. */

    /* Not used so far. Waiting for process interface. */

    // struct timespec i_atime;    /* Time of last file access. */
    // struct timespec i_mtime;    /* Time of last file write. */
    // struct timespec i_ctime;    /* Time of last inode change. */
} inode_t;


/**
 * @brief Create a new disk inode for a regular file 
 * associated with a dentry object in some directory.
 * 
 */
// void create()


#endif /* _INODE_H */
