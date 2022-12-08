/**
 * @file ps.c
 * @brief ps displays information about a selection of the active processes. 
 * ps selects all processes with the same effective user ID (euid=EUID) 
 * as the current user and associated with the same terminal as the invoker.  
 * It displays the process ID (pid=PID), the terminal associated with the 
 * process (tname=TTY).
 * @version 0.1
 * @date 2022-11-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFSIZE     128
#define NUMPROC     100
#define HEADERY     6
#define HEADERX     32
#define NSPACE      5



void print_stat(int nproc, char *info[]) {
    int i;
    const char header[HEADERY][HEADERX] = { 
        "PID", "PPID", "CMD", "NICE", "STATE", "RUNTIME" 
    };


    for (i = 0; i < HEADERY; ++i) {
        printf("%s", header[i]);
        if (i != HEADERY - 1)
            printf("     ");
    }
    printf("\n");

    // for (i = 0; i < nproc; ++i) {
    //     printf("%s\n", info[i]);

    // }
}


int main(void) {
    int i;
    int nproc;
    int sum_proc = NUMPROC;

    printf("????\n");
    char **info = malloc(sum_proc * sizeof(char*));

    for (i = 0; i < sum_proc; ++i) {
        info[i] = malloc(BUFSIZE * sizeof(char));
    }

    // while(1) {
        nproc = stat(info);
        print_stat(nproc, info);

        // if (nproc >= sum_proc / 2) {

        // }
    // }

    return 0;
}

