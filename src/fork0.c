#include <unistd.h>
#include <stdio.h>

int main(void) {
    pid_t pid;

    if ((pid = fork()) == -1) {
        printf("fork failed!\n");
        return 1;
    }
    
    if (pid) {
        /* parent process running */
        fputs(stdout, "Hello from parent!\n");
    } else {
        /* child process running */
        fputs(stdout, "Hello from child!\n");
    }

    /* this string should be printed twice */
    printf("Fork success!\n");
    return 0;
}
