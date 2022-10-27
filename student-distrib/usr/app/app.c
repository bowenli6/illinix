#include <stdio.h>
#include "string.h"
#include <unistd.h>


int abs(int a) {
    return a? a : -a;
}

int main(void) {
    char buf1[10];
    char buf2[10];
    // while (1) {
    //     ssize_t x = read(0, (void*)buf1, 5);
    //     write(1, (void*)buf1, 10);
    //     memset((void*)buf1, 0, 10);   
    // }

    printf("%d\n", abs(12));
    printf("%d\n", abs(-12));


    // strcpy(buf1, "abc");
    // printf("%s\n", buf1);
    // ssize_t y = write(1, buf1, 6);
    // printf("\nsize=%ld\n", y);

    // double p[3];
    // printf("%p\n", p);
    // printf("%p\n", p+1);
    // ssize_t y = read(0, (void*)buf2, 10);
    // printf("%ld\n", y);
    // printf("%s\n", buf2);
    return 0;
}