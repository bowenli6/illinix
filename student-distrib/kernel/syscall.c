#include <boot/syscall.h>
#include <pro/process.h>
#include <io.h>

/**
 * @brief Validate the input address inside the process address space
 * 
 * @param addr : A pointer points to the space to check
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
int32_t validate_addr(void *addr) {
   /* There are two possible ways to perform this check: 
    * (Perfer the 2nd one if you can! The first type of checking is quite time
    * consuming because it must be executed for each address parameter included
    * in a system call)
    * 
    * (1). Verify that the linear address belongs to the process address space
    * and, if so, that the memory region including it has the proper access rights.
    * 
    * (2). Verify just that the linear address is lower than PAGE_OFFSET (i.e., that
    * it does not fall within the range of interval address reserved to the kernel).
    * 
    */

   /* Implmentation: */
   uint32_t a = (uint32_t) addr;
   uint32_t size = (uint32_t) MAXADDR;
   if (a + size < a ||
       a + size > curr_process->addr.limit.seg)
       return 0;
   return -EPERM;
}

asmlinkage int32_t sys_halt(uint8_t status) {
   return 0;
}

asmlinkage int32_t sys_execute(const uint8_t *cmd) {
   return 0;
}

asmlinkage int32_t sys_getargs(uint8_t *buf, int32_t nbytes) {
   return 0;
}

asmlinkage int32_t sys_vidmap(uint8_t **screen_start) {
   return 0;
}

asmlinkage int32_t sys_set_handler(int32_t signum, void *handler_addr) {
   return 0;
}

asmlinkage int32_t sys_sigreturn(void) {
   return 0;
}
