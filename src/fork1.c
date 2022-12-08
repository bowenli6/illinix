#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


/**
 * @expected:
 * parent: x=0
 * child: x=2
 */
int main(void) {
    pid_t pid;
    int x = 1;

    pid = Fork();
    if (pid == 0) { /* Child */
        printf("child : x=%d\n", ++x);
        exit(0);
    }

    /* Parent */
    printf("parent: x=%d\n", --x);
    exit(0);
}
