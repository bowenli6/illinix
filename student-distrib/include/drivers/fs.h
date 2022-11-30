#ifndef _FS_H
#define _FS_H
 
#include <types.h>

#define FILES_MAX   63          /* Totally 63 files can be stored in this file system. */
#define BLOCK_SIZE  4096        /* Each block is 4KB. */
#define NAMESIZE    32          /* The file name of a file is up to 32 bytes. */

typedef enum {
    RTC,                        /* Real-time clock. */
    DIRECTORY,                  /* Directory. */
    REGULAR,                    /* Regular file. */
    TERMINAL                    /* Terminal. */
} file_type_t;


/* The metadate about a file. */
/* sizeof(dentry_t) == 64 */
typedef struct {
    int8_t fname[NAMESIZE];     /* File name. */
    file_type_t type;           /* File type. */
    uint32_t inode;             /* The index of inode, only meaningful for regular files. */     
    uint8_t reserved[24];       /* 24 bytes are reserved. */
} dentry_t; 


/* The first block of the file system memory. */
/* sizeof(boot_block) == 4096 */
typedef struct {
    uint32_t n_dir;             /* Number of the directory entries. */
    uint32_t n_inode;           /* Number of inodes. */
    uint32_t n_datab;           /* Number of data blocks. */
    uint8_t  reserved[52];      /* 52 bytes are reserved. */
    dentry_t dirs[63];          /* 63 directory entries left. */
} boot_block;


/* A inode stores general information about a specific file. */
/* sizeof(inode_t) = 4096 bytes. */
typedef struct {
    uint32_t size;              /* Number of bytes in the file. */
    uint32_t data_block[1023];  /* The data blocks used in the file. */
    /* 1023 because the total size of inode_t is 4096.*/
} inode_t;


typedef struct {
    int8_t data[BLOCK_SIZE];
} data_block;


typedef struct {
    int idx;                            /* The index we want to start at a given data block. */
    int nblock;                         /* The nth block to read. (NOT data block index) */
    uint32_t iblock;                    /* The index of the data_block. */
    data_block *datab;                  /* The data block used by the offset. */
} virtual_pos;


typedef struct {
    boot_block *boot;                   /* The first block of the file system. */
    inode_t *inodes;                    /* The address of the statring of the inodes block, up to 63 inodes (1st is the '.' directory). */
    data_block *data_block_addr;        /* The address of the statring data block. */
} fs_t;


extern fs_t *fs;

void fs_init(uint32_t start_addr);
int32_t read_dentry_by_name(const int8_t *fname, dentry_t *dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length);
uint32_t get_size(uint32_t index);

#endif /* _FS_H */
