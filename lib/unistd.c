#include <unistd.h>

/* process */

pid_t fork(void) {

}


void exit(int status) {

}


// pid_t waitpid(pid_t pid, int *wstatus);

pid_t getpid(void) {

}

// pid_t getppid(void);


/* file system */
// int open(const char *pathname);
// int close(int fd);
// ssize_t read(int fd, void *buf, size_t count);
// int close(int fd);


/* memory management */
// int brk(void *addr);
// void *sbrk(size_t increment);

