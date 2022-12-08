#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


int main(void) {
    int i;
    pid_t pid;
    const int argsize = 10;
    const int numarg = 2;
    char **argv = malloc(numarg * sizeof (char *));

    for (i = 0; i < numarg; ++i) 
        argv[i] = malloc(argsize * sizeof(char));

    fgets(argv[0], argsize, stdin);

    if ((pid = Fork()) == 0) { 
        /* Child runs program */
        if (execv(argv[0], (char *const *)argv) < 0) {
            printf("%s: Command not found.\n", argv[0]);
            exit(1);
        }
    }

    exit(0);
}
