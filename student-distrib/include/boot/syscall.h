#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <types.h>

#define SYSCALL 0x80
#define asmlinkage __attribute__((regparm(0)))

void syscall_handler();

/* Required by ECE391. */
asmlinkage void sys_exit(uint8_t status);
asmlinkage int32_t sys_execute(const int8_t *cmd);
asmlinkage int32_t sys_open(const int8_t *filename);
asmlinkage int32_t sys_close(int32_t fd);
asmlinkage int32_t sys_read(int32_t fd, void *buf, uint32_t nbytes);
asmlinkage int32_t sys_write(int32_t fd, const void *buf, uint32_t nbytes);
asmlinkage int32_t sys_getargs(uint8_t *buf, int32_t nbytes);
asmlinkage int32_t sys_vidmap(uint8_t **screen_start);
asmlinkage int32_t sys_set_handler(int32_t signum, void *handler_addr);
asmlinkage int32_t sys_sigreturn(void);


/* Extra Credit */

asmlinkage int32_t sys_restart(void);
asmlinkage int32_t sys_fork(void);
asmlinkage int32_t sys_execv(const int8_t *pathname, int8_t *const argv[]);
asmlinkage int32_t sys_getpid(void);
asmlinkage int32_t sys_getppid(void);
asmlinkage int32_t sts_wait(int *wstatus);
asmlinkage int32_t sys_waitpid(pid_t pid, int *wstatus);
asmlinkage void   *sys_sbrk(uint32_t size);
asmlinkage int32_t sys_mmap(void *addr, uint32_t size);
asmlinkage int32_t sys_munmap(void *addr);
asmlinkage int32_t sys_stat(int8_t *info[]);



/* --------------------------------Process-------------------------------- */

// asmlinkage int32_t sys_fork(void);
// asmlinkage int32_t sys_exit(uint8_t status);
// asmlinkage int32_t sys_exec(const char *file, char *const argv[], char *const envp[]);
// asmlinkage int32_t waitpid(pid_t pid, int *wstatus, int options);
// asmlinkage int32_t getpid(void);
// asmlinkage int32_t getppid(void);
// asmlinkage int32_t getuid(void);
// asmlinkage int32_t getgid(void);
// asmlinkage int32_t kill(pid_t pid, int sig);
// asmlinkage int32_t signal(int signum, sighandler_t handler);
// asmlinkage int32_t alarm(unsigned int seconds);


// /* -----------------------------Virtual Memory----------------------------- */
// int brk(void *addr);
// void *sbrk(int32_t increment);
// void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) 
// int munmap(void *addr, size_t length);


// /* ------------------------------File system------------------------------ */

// /* open files */
// int open(const char *pathname, int flags);
// int open(const char *pathname, int flags, mode_t mode);
// int creat(const char *pathname, mode_t mode);

// /* close files */
// int close(int fd);

// /* read & write files */
// int read(int fd, void *buf, uint32_t count);
// int write(int fd, const void *buf, uint32_t count);

// /* random access */
// off_t lseek(int fd, off_t offset, int whence);

// /* Access Control */
// int access(const char *pathname, int mode);
// int chmod(const char *pathname, mode_t mode);
// int fchmod(int fd, mode_t mode);
// int chown(const char *pathname, uid_t owner, gid_t group);
// int fchown(int fd, uid_t owner, gid_t group);
// int lchown(const char *pathname, uid_t owner, gid_t group);
// int dup(int oldfd);
// int dup2(int oldfd, int newfd);

// /* hard linking */
// int link(const char *oldpath, const char *newpath);
// int unlink(const char *pathname);

// /* file information */
// int stat(const char *pathname, struct stat *statbuf);
// int fstat(int fd, struct stat *statbuf);
// int lstat(const char *pathname, struct stat *statbuf);

// /* devices control */
// int ioctl(int fd, unsigned long request, ...);

#endif /* _SYSCALL_H */
