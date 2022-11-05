#include <boot/syscall.h>
#include <ece391_vfs.h>
#include <pro/process.h>


/**
 * @brief A system call service routine for opening a file.
 * The calling convation of this function is to use the 
 * arguments from the stack.
 * 
 * @param filename : A file name
 * @return int32_t : positive or 0 denote success, negative values denote an error condition.
 */
asmlinkage int32_t sys_open(const uint8_t *filename) {
   return 0;
}


/**
 * @brief A system call service routine for closing a file.
 * The calling convation of this function is to use the 
 * arguments from the stack.
 * 
 * @param fd : The file descriptor of the file we want to close.
 * @return int32_t : positive or 0 denote success, negative values denote an error condition.
 */
asmlinkage int32_t sys_close(int32_t fd) {
   return file_close(fd);
}


/**
 * @brief A system call service routine for reading a file.
 * The calling convation of this function is to use the 
 * arguments from the stack.
 * 
 * @param fd : The file descriptor of the file we want to read.
 * @param buf : A buffer array that copys the content from the file.
 * @param nbytes The number of bytes to read from the file.
 * @return int32_t : positive or 0 denote success, negative values denote an error condition.
 */
asmlinkage int32_t sys_read(int32_t fd, void *buf, uint32_t nbytes) {
   return 0;
}


/**
 * @brief A system call service routine for writing a file.
 * The calling convation of this function is to use the 
 * arguments from the stack.
 * 
 * @param fd : The file descriptor of the file we want to write.
 * @param buf : A buffer array that copys the content to the file.
 * @param nbytes The number of bytes to write to the file. *
 * @return int32_t : positive or 0 denote success, negative values denote an error condition.
 */
asmlinkage int32_t sys_write(int32_t fd, const void *buf, uint32_t nbytes) {
   return 0;
}