#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFSIZE  32


/* NOT VERY USEFUL -.- */
int main(void) {
    int i;
    int ntasks;
    pid_t *pids;
    char *buffer = calloc(BUFSIZE, sizeof(char));

    while (1) {
        printf("Please pick a number of tasks to run:\n");

        /* get input from user */
        if (!fgets(buffer, BUFSIZE, stdin))
            printf("read command-line inputs failed!\n");
        
        if (!strcmp(buffer, "exit"))
            break;

        /* get number of tasks running */
        ntasks = atoi(buffer);

        pids = calloc(ntasks, sizeof(pid_t));

        printf("tasks staring forking and running ...\n");

        for (i = 0; i < ntasks; i++) {
            if ((pids[i] = Fork()) == 0) {
                /* child */
                while(1) printf("%d\n", i);
            }
        }
    }
    exit(0);
}
