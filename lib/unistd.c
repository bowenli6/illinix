#include <unistd.h>


/**
 * @brief Creates a new process by duplicating the calling process. 
 * The new process is referred to as the child process. The calling 
 * process is referred to as the parent process. The child process
 * and the parent process run in separate memory spaces. At the time 
 * of fork() both memory spaces have the same content.
 * 
 * @return pid_t : On success, the PID of the child process is 
 * returned in the parent, and 0 is returned in the child. 
 * On failure, -1 is returned in the parent, no child process is created, 
 * and errno is set appropriately.
 */
pid_t fork(void) {
    return (pid_t) syscall(SYS_FORK, 0, 0, 0);
}


/**
 * @brief Executes the program referred to by pathname. This causes 
 * the program that is currently being run by the calling process to 
 * be replaced with a new program, with newly initialized stack, heap, 
 * and (initialized and uninitialized) data segments.
 * 
 * @param pathname : file name of the new program
 * @param argv : an array of pointers to strings passed to the new 
 * program as its command-line arguments. By convention, the first of 
 * these strings (i.e., argv[0]) should contain the filename associated 
 * with the file being executed. The argv array must be terminated by a 
 * NULL pointer. (Thus, in the new program, argv[argc] will be NULL.)
 * 
 * @return int : On success, execve() does not return, on error -1 is
 * returned, and errno is set appropriately.
 */
int execv(const char *pathname, char *const argv[]) {
    return syscall(SYS_EXECV, (int) pathname, (int) argv, 0);
}


/**
 * @brief Create a new process and then replace it with a new program.
 * 
 * @param cmd : command-line arguments
 * @return int : On success, return status from the child process, on 
 * error -1 is returned, and errno is set appropriately.
 */
int execute(const char *cmd) {
    return syscall(SYS_EXECUTE, (int) cmd, 0, 0);
}


/**
 * @brief Terminates the calling process "immediately". 
 * Any open file descriptors belonging to the process are closed. 
 * The process's parent is sent a SIGCHLD signal.
 * 
 * @param status : The value status & 0xFF is returned to the 
 * parent process as the process's exit status, and can be collected 
 * by the parent using one of the wait(2) family of calls.
 */
void _exit(int status) {
    syscall(SYS_EXIT, status, 0, 0);
}


/**
 * @brief Suspends execution of the calling thread until one of its 
 * children terminates. The call wait(&wstatus) is equivalent to: 
 * waitpid(-1, &wstatus, 0)
 * 
 * @param pid 
 * @param wstatus 
 * @return pid_t : On success, returns the process ID of the terminated 
 * child; On error, -1 is returned.

 */
pid_t wait(int *wstatus) {
    return (pid_t) syscall(SYS_WAIT, (int) wstatus, 0, 0);
}


/**
 * @brief Suspends execution of the calling thread until a child 
 * specified by pid argument has changed state.
 * 
 * @param pid : pid of the waiting child thread
 * @param wstatus : 
 * @return pid_t : On success, returns the process ID of the child 
 * whose state has changed; On error, -1 is returned.
 */
pid_t waitpid(pid_t pid, int *wstatus) {
    return (pid_t) syscall(SYS_WAITPID, (int) pid, (int) wstatus, 0);
}



/**
 * @brief Get the process ID (PID) of the calling process.
 * 
 * @return pid_t : pid of the calling process.
 */
pid_t getpid(void) {
    return (pid_t) syscall(SYS_GETPID, 0, 0, 0);
}


/**
 * @brief Returns the process ID of the parent of the calling process. 
 * This will be either  the ID of the process that created this process 
 * using fork(), or, if that process has already terminated, the ID of 
 * the process to which this process has been reparented (init process).
 * 
 * @return pid_t : pid of the parent of the calling process.
 */
pid_t getppid(void) {
    return (pid_t) syscall(SYS_GETPPID, 0, 0, 0);
}



/**
 * @brief Stores the program arguments of the running process into buf
 * 
 * @param buf : arguments buffer
 * @param nbytes : number of bytes of the argument
 * @return int : On success, returns 0; On error, -1 is returned.
 */
int getargs (char* buf, int nbytes) {
    return syscall(SYS_GETARGS, (int) buf, nbytes, 0);
}


/**
 * @brief Opens the file specified by pathname.
 * 
 * @param pathname : name of the file
 * @return int : a file descriptor, a small, nonnegative integer that 
 * is used in subsequent system calls (read(2), write(2), lseek(2), 
 * fcntl(2), etc.) to refer to the open file. The file descriptor returned 
 * by a successful call will be the lowest-numbered file descriptor not 
 * currently open for the process.
 * 
 */
int open(const char *pathname) {
    return syscall(SYS_READ, (int) pathname, 0, 0);
}



/**
 * @brief Closes a file descriptor, so that it no longer refers to 
 * any file and may be reused.
 * 
 * @param fd : file descriptor
 * @return int : returns zero on success. On error, -1 is returned, 
 * and errno is set appropriately
 */
int close(int fd) {
    return syscall(SYS_CLOSE, fd, 0, 0);
}



/**
 * @brief Attempts to read up to count bytes from file descriptor fd 
 * into the buffer starting at buf.
 * 
 * @param fd : file descriptor
 * @param buf : read buffer
 * @param count : number of bytes to be read
 * @return ssize_t : On success, the number of bytes read is returned 
 * (zero indicates end of file),  and the file position is advanced by 
 * this number.  It is not an error if this number is smaller than the 
 * number of bytes requested; this may happen for example because fewer 
 * bytes are actually available right now (maybe because we were close 
 * to end-of-file, or because we are reading from a pipe, or from a terminal), 
 * or because read() was interrupted by a signal.
 */
ssize_t read(int fd, void *buf, size_t count) {
    return (ssize_t) syscall(SYS_READ, fd, (int) buf, (int) count);
}   



/**
 * @brief Writes up to count bytes from the buffer starting at buf to the 
 * file referred to by the file descriptor fd.
 * 
 * @param fd : file descriptor
 * @param buf : read buffer
 * @param count : number of bytes to write 
 * @return ssize_t : On success, the number of bytes written is returned.  
 * On error, -1 is returned, and errno is set to indicate the cause of the error.
 */
ssize_t write(int fd, const void *buf, size_t count) {
    return (ssize_t) syscall(SYS_WRITE, fd, (int) buf, (int) count);
}



/**
 * @brief Change the location of the program break, which defines 
 * the end of the process's data segment (i.e., the program break 
 * is the first location after the end of the uninitialized data 
 * segment). Increasing the program break has the effect of 
 * allocating memory to the process; decreasing the break 
 * deallocates memory.
 * 
 * Calling sbrk() with an increment of 0 can be used to find the 
 * current location of the program break.
 * 
 * @param increment : increments the program's data space by 
 * increment bytes.
 * 
 * @return void* : On success, sbrk() returns the previous program 
 * break.  
 * (If thebreak 
 * was increased, hen this value is a pointer to the start of the 
 * newly allocated memory).  On error, (void *) -1 is returned, and 
 * errno is set to ENOMEM.
 */
void *sbrk(size_t increment) {
    return (void *) syscall(SYS_SBRK, (int) increment, 0, 0);
}



/**
 * @brief Mapping the text-mode video memory into user space at a 
 * pre-set virtual address.
 * 
 * @param screen_start : starting screen address
 * @return int : positive or 0 denote success, negative values denote an error condition
 */
int vidmap(char **screen_start) {
    return syscall(SYS_VIDMAP, (int) screen_start, 0, 0);
}


/**
 * @brief Creates a new mapping in the virtual address space of 
 * the calling process.  
 * 
 * If addr is NULL, then the kernel chooses the (page-aligned) 
 * address at which to create the mapping; this is the most portable 
 * method of creating a new mapping. 
 * 
 * @param addr : starting address for the new mapping is specified 
 * in addr. 
 * @param size : specifies the length of the mapping (which must be 
 * greater than 0).
 * 
 * @return void* : On success, mmap() returns a pointer to the 
 * mapped area. On error, the value MAP_FAILED 
 * (that is, (void *) -1) is returned, and errno is set to indicate 
 * the cause of the error.
 */
void *mmap(void *addr, size_t size) {
    return (void *) syscall(SYS_MMAP, (int) addr, (int) size, 0);
}



/**
 * @brief Unmap the virtual address space of the calling process
 * 
 * @param addr : starting address for the new mapping is specified 
 * in addr. 
 * @param size : specifies the length of the mapping (which must be 
 * greater than 0).
 * @return int : On  success,  munmap() returns 0. On failure, it
 * returns -1, and errno is set to indicate the cause of the error 
 * (probably to EINVAL).
 */
int munmap(void *addr, size_t size) {
    return syscall(SYS_MUNMAP, (int) addr, (int) size, 0);
}


int stat(char *info[]) {
    return syscall(SYS_STAT, (int) info, 0, 0);
}
