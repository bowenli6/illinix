#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <types.h>


#define SYSCALL 0x80
// #define asmlinkage __attribute__((regparm(0)))

void syscall_handler();


/* Required by ECE391. */
 int32_t sys_halt(uint8_t status);
 int32_t sys_exe(const uint8_t *cmd);
 int32_t sys_open(const uint8_t *filename);
 int32_t sys_close(int32_t fd);
 int32_t sys_read(int32_t fd, void *buf, uint32_t nbytes);
 int32_t sys_write(int32_t fd, const void *buf, uint32_t nbytes);
// int32_t getargs(uint8_t *buf, int32_t nbytes);
// int32_t vidmap(uint8_t **screen_start);
// int32_t set_handler(int32_t signum, void *handler_addr);
// int32_t sigreturn(void);



// /* --------------------------------Process-------------------------------- */


// /* process creation */
// pid_t fork(void);
// int exec(const char *file, char *const argv[], char *const envp[]);

// /* termination */
// pid_t wait(int *wstatus);
// pid_t waitpid(pid_t pid, int *wstatus, int options);

// /* identity */
// pid_t getpid(void);
// pid_t getppid(void);
// uid_t getuid(void);
// gid_t getgid(void);

// /* control */
// int kill(pid_t pid, int sig);
// sighandler_t signal(int signum, sighandler_t handler);
// unsigned int alarm(unsigned int seconds);


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
