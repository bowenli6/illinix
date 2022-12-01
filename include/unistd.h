#ifndef _UNISTD_H_
#define	_UNISTD_H_

#include <type.h>


/* process */
pid_t fork(void);
int exec(const char *pathname, const char *arg);
void exit(int status);
int execute(const char *arg);
int getargs (char* buf, int nbytes);
pid_t waitpid(pid_t pid, int *wstatus);
pid_t getpid(void);
pid_t getppid(void);


/* file system */
int open(const char *pathname);
int close(int fd);
ssize_t read(int fd, void *buf, size_t count);
int close(int fd);


/* memory management */
int brk(void *addr);
void *sbrk(size_t increment);
int vidmap(char **screen_start);


/* signal */



#endif /* _UNISTD_H_ */
