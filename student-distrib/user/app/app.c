#include "app.h"
#include <stdio.h>
#include "string.h"

/* A inode stores general information about a specific file. */
/* sizeof(inode_t) = 4096 bytes. */
typedef struct {
    unsigned int size;              /* Number of bytes in the file. */
    unsigned int data_block[1023];  /* The data blocks used in the file. */
    /* 1023 because the total size of inode_t is 4096.*/
} inode_t;

int main(void) {
    dentry_t fd;
    boot_block block;
   
    printf("%d\n", (int) sizeof(inode_t));
    return 0;
}