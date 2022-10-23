#include <stdio.h>
#include "string.h"
#include <unistd.h>

/* A inode stores general information about a specific file. */
/* sizeof(inode_t) = 4096 bytes. */
typedef struct {
    unsigned int size;              /* Number of bytes in the file. */
    unsigned int data_block[1023];  /* The data blocks used in the file. */
    /* 1023 because the total size of inode_t is 4096.*/
} inode_t;

int main(void) {
    char buf1[10];
    char buf2[10];
    // while (1) {
    //     ssize_t x = read(0, (void*)buf1, 3);
    //     printf("%ld\n", x);
    //     printf("%s\n", buf1);
    //     memset((void*)buf1, 0, 10);   
    // }

    // strcpy(buf1, "abc");
    // printf("%s\n", buf1);
    // ssize_t y = write(1, buf1, 6);
    // printf("\nsize=%ld\n", y);

    double p[3];
    printf("%p\n", p);
    printf("%p\n", p+1);
    // ssize_t y = read(0, (void*)buf2, 10);
    // printf("%ld\n", y);
    // printf("%s\n", buf2);
    return 0;
}