#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int main(void) {
    unsigned long pid;

    if ((pid = ece391_fork()) == -1) {
        ece391_fdputs(1, (uint8_t*)"fork failed!\n");
        return 1;
    }
    
    if (pid) {
        /* parent process running */
        ece391_fdputs(1, (uint8_t*)"Hello from parent!\n");
    } else {
        /* child process running */
        ece391_fdputs(1, (uint8_t*)"Hello from child!\n");
    }

    /* this string should be printed twice */
    ece391_fdputs(1, (uint8_t*)"Fork success!\n");
    return 0;
}
