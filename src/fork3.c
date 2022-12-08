#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


/**
 * @expected:
 * L0                       L0
 * L1                       L1
 * Bye          OR          L2 
 * L2                       Bye
 * Bye                      Bye
 * Bye                      Bye
 *
 *
 */
int main(void) {
    printf("L0\n");
    if (fork() != 0) {
        printf("L1\n");
        if (fork() != 0) {
            printf("L2\n");
        }
    }
    printf("Bye\n");
    exit(0);
}