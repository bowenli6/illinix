#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define MAXARGS 10

int parse_arg(char *cmd, char *argv[]) {
    char *delim;                    /* points to the first space delimiter */
    int argc;                       /* number of arguments */
    char buf[strlen(cmd) + 2];    /* buffer that contains command line */

    if (!cmd) return -1;

    strcpy(buf, cmd);
    cmd = buf;

    /* replace trailing \n with space */
    if ((cmd[strlen(cmd) - 1] == '\n') || (cmd[strlen(cmd) - 1] == '\r'))
        cmd[strlen(cmd) - 1] = ' ';
    else
        cmd[strlen(cmd)] = ' ';

    /* skipping leading spaces */
    while (*cmd && (*cmd == ' ')) ++cmd;

    /* build the argv list */
    argc = 0;
    while ((delim = strchr(cmd, ' '))) {
        if (argc == MAXARGS) return -1;
        /* copy argument */
        *delim = '\0';
        strcpy(argv[argc++], cmd);
        cmd = delim + 1;

        /* skipping leading spaces */
        while (*cmd && (*cmd == ' ')) ++cmd;
    }

    /* blank line */
    if (!argc) return -1;

    argv[argc] = NULL;
    
    return argc;
}


int main(void) {
    int i;
    pid_t pid;
    const int argsize = 10;
    const int numarg = 2;
    char buf[128];
    char **argv = malloc(numarg * sizeof (char *));

    for (i = 0; i < numarg; ++i) 
        argv[i] = malloc(argsize * sizeof(char));

    fgets(buf, 128, stdin);

    parse_arg(buf, argv);

    if ((pid = Fork()) == 0) { 
        /* Child runs program */
        if (execv(argv[0], (char *const *)argv) < 0) {
            printf("%s: Command not found.\n", argv[0]);
            exit(1);
        }
    }

    exit(0);
}
