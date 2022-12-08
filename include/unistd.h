#ifndef _UNISTD_H_
#define	_UNISTD_H_

#include <type.h>

/* system call number */
typedef enum {
    SYS_RESTART,
    SYS_EXIT,
    SYS_EXECUTE,
    SYS_READ,
    SYS_WRITE,
    SYS_OPEN,
    SYS_CLOSE,
    SYS_GETARGS,
    SYS_VIDMAP,
    SYS_SET_HANDLER,
    SYS_SIGRETURN,
    SYS_FORK,
    SYS_EXECV,
    SYS_WAIT,
    SYS_WAITPID,
    SYS_GETPID,
    SYS_GETPPID,
    SYS_SBRK,
    SYS_MMAP,
    SYS_MUNMAP,
    SYS_STAT
} sysnum;


int syscall(sysnum sysnum, int arg0, int arg1, int arg2);

/* process */
pid_t fork(void);
void _exit(int status);
int execv(const char *pathname, char *const argv[]);
int execute(const char *cmd);
pid_t wait(int *wstatus);
pid_t waitpid(pid_t pid, int *wstatus);
pid_t getpid(void);
pid_t getppid(void);
int getargs (char* buf, int nbytes);

/* Debug */
int stat(char *info[]);


/* file system */
int open(const char *pathname);
int close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);

/* memory management */
void *sbrk(size_t increment);
int vidmap(char **screen_start);
void *mmap(void *addr, size_t size);
int munmap(void *addr, size_t size);



#endif /* _UNISTD_H_ */
