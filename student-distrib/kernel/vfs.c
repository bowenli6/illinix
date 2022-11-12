#include <boot/syscall.h>
#include <drivers/fs.h>
#include <drivers/rtc.h>
#include <vfs/ece391_vfs.h>
#include <pro/process.h>
#include <errno.h>
#include <access.h>
#include <lib.h>

static int32_t validate_fd(int32_t fd);
static int32_t validate_fname(const int8_t *filename);

/**
 * @brief A system call service routine for opening a file
 * The calling convation of this function is to use the 
 * arguments from the stack
 * 
 * @param filename : A file name
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
asmlinkage int32_t sys_open(const int8_t *filename) {
   int32_t errno;
   // uint8_t kbuf[NAMESIZE];

   /* validate file descriptor */
   if ((errno = validate_fname(filename)) < 0)
      return errno;

   /* copy data from user space to kernel space */
   // if ((errno = copy_from_user((void *)kbuf, (void *)filename, 
   //                            strlen(filename))) <= 0)
   if (*filename == '.') return directory_open(filename);
   if (!strcmp(filename, "rtc")) RTC_open(filename);
   return file_open(filename);
}


/**
 * @brief A system call service routine for closing a file
 * The calling convation of this function is to use the 
 * arguments from the stack
 * 
 * @param fd : The file descriptor of the file we want to close
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
asmlinkage int32_t sys_close(int32_t fd) {
   int32_t errno;
   file_op f_op;

   /* validate file descriptor */
   if ((errno = validate_fd(fd)) < 0)
      return errno;
   
   /* invoke close routine */
   f_op = CURRENT->fds.fd[fd].f_op;
   return f_op.close(fd); 
}


/**
 * @brief A system call service routine for reading a file
 * The calling convation of this function is to use the 
 * arguments from the stack
 * 
 * @param fd : The file descriptor of the file we want to read.
 * @param buf : A buffer array that copys the content from the file
 * @param nbytes The number of bytes to read from the file
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
asmlinkage int32_t sys_read(int32_t fd, void *buf, uint32_t nbytes) {
   int32_t errno;
   file_op f_op;
   // uint8_t kbuf[nbytes];

   /* validate file descriptor */
   if ((errno = validate_fd(fd)) < 0)
      return errno;

   /* Might be unsuitable for reading: return -EINVAL in the future */
   
   /* validate nbytes */
   if (nbytes < 0) return -1;

   /* copy data from user space to kernel space */
   // if ((errno = copy_from_user((void *)kbuf, (void *)buf, nbytes)) <= 0)
   //    return errno;

   /* copy data from user space to kernel space*/
   /* invoke read routine */
   f_op = CURRENT->fds.fd[fd].f_op;
   return f_op.read(fd, (void *)buf, nbytes);
}


/**
 * @brief A system call service routine for writing a file
 * The calling convation of this function is to use the 
 * arguments from the stack
 * 
 * @param fd : The file descriptor of the file we want to write
 * @param buf : A buffer array that copys the content to the file
 * @param nbytes The number of bytes to write to the file *
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
asmlinkage int32_t sys_write(int32_t fd, const void *buf, uint32_t nbytes) {
   int32_t errno;
   file_op f_op;
   // uint8_t kbuf[nbytes];

   /* validate file descriptor */
   if ((errno = validate_fd(fd)) < 0)
      return errno;

   /* Might be unsuitable for writing: return -EINVAL in the future */

   /* validate nbytes */
   if (nbytes < 0) return -1;

   /* copy data from user space to kernel space */
   // if ((errno = copy_from_user((void *)kbuf, (void *)buf, nbytes)) <= 0)
   //       return errno;

   /* invoke write routine */
   f_op = CURRENT->fds.fd[fd].f_op;
   return f_op.write(fd, (void *)buf, nbytes);
}



/**
 * @brief Validate a file descriptor
 * 
 * @param fd : a file descriptor
 * @return int32_t : 0 denote success, negative values denote an error condition
 */
static int32_t validate_fd(int32_t fd) {
   int max_fd = CURRENT->fds.max_fd;

   if (fd < 0 || fd >= max_fd) return -1;

   if (!CURRENT->fds.fd[fd].f_count) return -1;

   /* should check if the file has permission to access in the future */

   return 0;
}


/**
 * @brief Validate a file name
 * 
 * @param fd : a file name
 * @return int32_t : 0 denote success, negative values denote an error condition
 */
static int32_t validate_fname(const int8_t *filename) {
   if (!filename) return -1;

   if (strlen(filename) > NAMESIZE) return -1;

   return 0;
}
