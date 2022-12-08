#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


/**
 * @expected:
 * L0
 * L1
 * Bye
 * L1
 * Bye
 * Bye
 * Bye
 */
int main(void) {
    printf("L0\n");
    Fork();
    printf("L1\n");
    Fork();
    printf("Bye\n");
    exit(0);
}