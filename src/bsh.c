/**
 * @file bsh.c
 * @author Bowen Li (bowenli6@illinois.edu)
 * @brief bsh (The B shell) is a UNIX command interpreter (shell) usable 
 * as an interactive login shell and as a shell script command processor
 * that executes commands read from the standard input or from a file.  
 * bsh also incorporates useful features from the Bourne-Again, Z, Korn 
 * and C shells (bash, zsh, ksh and csh).
 * @version 0.1
 * @date 2022-11-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


#define MAXARGS 10              /* Max number of arguments supported in bsh */
#define MAXUSER 32              /* Max number of bytes a user name can be */
#define MAXLINE 256             /* Max number of bytes a command line can hold */
#define MAXDIR  256             /* Max number of bytes a directory name can be */


/* local function prototypes */
static int parse(char *buf, char *argv[]);
static int eval(char *cmd);
static int buildin(char *argv[]);
static void echo(char *argv[]);


int main(void) {
    char user[MAXUSER];         /* user name of this process */
    char dir[MAXDIR];           /* current directory name */
    char cmdline[MAXLINE];      /* command line */

    /* get the user name */
    strcpy(user, "root");
    // TODO

    /* get the current directory */
    strcpy(dir, "~");
    // TODO

    /* REPL: read eval print loop */
    while (1) {
        /* print */
        printf("%s@illinix %s %% ", user, dir);

        /* read */
        if (!fgets(cmdline, MAXLINE, stdin)) {
            printf("Read command line failed!\n");
            continue;
        }
        
        // if (feof(stdin)) 
        //     exit(0);
    
        /* eval */
        if (eval(cmdline))
            /* ask for current directory again */
            // TODO
            continue;

        /* add more feature in the future */
        // TODO
    }
}


/**
 * @brief Evaluate and execute a command from the shell input line.
 * 
 * @param cmd : command line
 * @return int : 1 if the command is cd, 0 otherwise
 */
static int eval(char *cmd) {
    char *argv[MAXARGS];    /* argument list for exec() */
    char buf[MAXLINE];      /* holds modified command line */
    int background;         /* does the process run in background? */
    pid_t pid;              /* process id */
    // int status;             /* wait process status */
    
    /* parse */
    strcpy(buf, cmd);
    background = parse(buf, argv);

    /* empty command */
    if (*argv == NULL) return 0;

    /* buildin command executes and exits */
    if (buildin(argv)) return 0;

    /* not buildin command, fork a new process */
    if (!(pid = Fork())) {
        /* child process */
        Execv(argv[0], argv);
    } else {    
        /* parent process */
        if (background) {
            printf("%d is executing %s in background", pid, cmd);
        } else {
            /* parent waits for foreground process to terminate */
            // not implemened yet due to lack of signals
            // Waitpid(pid, &status);
        }
    }
    if (!strcmp(argv[0], "cd"))
        return 1;
    return 0;
}



/**
 * @brief parse the command line
 * 
 * @param buf : copy of the command line
 * @param argv : arguments list to be set
 * @return int : 1 if the process run in backgroud
 *               0 otherwise
 */
static int parse(char *buf, char *argv[]) {
    char *delim;        /* points to the first space delimiter */
    int argc;           /* number of arguments */
    int background;     /* does the process run in background? */

    /* replace trailing \n with space */
    buf[strlen(buf) - 1] = ' ';

    /* skipping leading spaces */
    while (*buf && (*buf == ' ')) ++buf;

    /* build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        /* copy argument */
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;

        /* skipping leading spaces */
        while (*buf && (*buf == ' ')) ++buf;
    }
    argv[argc] = NULL;

    /* blank line */
    if (!argc) return 1;
    
    /* should the process run in the background? */
    if ((background = (*argv[argc - 1] == '&')))
        argv[--argc] = NULL;
    
    return background;
}


/**
 * @brief check if the command is buildin from the bsh
 * 
 * @param argv : argument list
 * @return int 1 if is buildin, 0 otherwise
 */
int buildin(char *argv[]) {
    /* exit from the shell process */
    if (!strcmp(*argv, "exit"))
        exit(0);
    
    /* ignore single & */
    if (!strcmp(*argv, "&"))
        return 1;

    /* echo */

    if (!strcmp(*argv, "echo"))
        echo(argv + 1);
    

    /* add more */
    // TODO

    /* otherwise not buildin command */
    return 0;
}


/**
 * @brief build-in function
 * 
 * @param buffer 
 */
static void echo(char *argv[]) {
    while (*argv) {
        printf("%s");
        if (*(argv + 1)) 
            printf(" ");
    }
}
