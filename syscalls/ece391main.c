#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, const char *argv[]) {
    pid_t pid, child;
    int status;
    fprintf(stdout, "argc: %d\n", argc);
    for (int i = 0; i < argc; ++i) {
        fprintf(stdout, "argv[%d]: %s\n", i, argv[i]);
    }
    
    if ((child = fork()) < 0) {
        fprintf(stderr, "fork failed!\n");
        printf("\n");
    }

    if (child == 0) {
        execlp("ls", "ls", (char *)0);
    } else {
       if ((pid = waitpid(child, &status, 0)) < 0) {
            fprintf(stderr, "waitpid failed!\n");
       }
    }
    exit(0);
}   