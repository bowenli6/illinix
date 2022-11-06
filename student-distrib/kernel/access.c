#include <access.h>
#include <pro/process.h>

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

   /* Implmentation */
   uint32_t a = (uint32_t) addr;
   uint32_t size = (uint32_t) MAXADDR;

   if (a + size < a ||
       a + size > curr_process->addr.limit.seg)
       return 0;
       
   return -EFAULT;
}

/**
 * @brief Copies a block of arbitrary size from user space.
 * 
 * Side effect: The pointer from is an untrusted pointer 
 * (a pointer that may be invalid in the current virtual address mapping)
 * 
 * @param to : Target address from user space
 * @param from : Source address from kernel space
 * @param n : Number of bytes to transfer
 * @return int32_t : number of bytes denote success, 0 or negative values denote an error condition
 */
int32_t copy_from_user(void *to, const void *from, uint32_t n) {
    int errno;

    if ((errno = validate_addr(from)) < 0) 
        return errno;
    
    /* perform transfer of data */
    return 0;
}


/**
 * @brief Copies a block of arbitrary size to user space.
 * 
 * Side effect: The pointer to is an untrusted pointer 
 * (a pointer that may be invalid in the current virtual address mapping)
 * 
 * @param to : Target address from kernel space
 * @param from : Source address from user space
 * @param n : Number of bytes to transfer
 * @return int32_t : number of bytes denote success, 0 or negative values denote an error condition
 */
int32_t copy_to_user(void *to, const void *from, uint32_t n) {
    int errno;

    if ((errno = validate_addr(to)) < 0) 
        return errno;
    
    /* perform transfer of data */
    return 0;
}


