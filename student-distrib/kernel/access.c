#include <access.h>
#include <pro/process.h>
#include <errno.h>
#include <boot/x86_desc.h>
#include <boot/page.h>
#include <lib.h>
#include <kmalloc.h>


// static int32_t validate_addr(void *addr);


/**
 * @brief map user virtual memory to process pid's physical memory
 * 
 * @param pid process id (start at 2)
 */
void user_mem_map(pid_t pid) {
    page_directory[VIR_MEM_BEGIN >> PDE_OFFSET_4MB] = PTE_PRESENT | PTE_RW
         | PTE_US | PDE_MB | INDEX_TO_DIR(pid + 14);
    flush_tlb();
}

/**
 * @brief user_mem_unmap
 * 
 * @param pid process id (start at 2)
 */
void user_mem_unmap(pid_t pid) {
    page_directory[VIR_MEM_BEGIN >> PDE_OFFSET_4MB] = 0;
    flush_tlb();
}

/**
 * @brief Alloc a 8KB memory in kernel for process pid
 * 
 * @param pid process id (start at 2)
 * @return void* pointer to the process kernel stack
 */
void *alloc_kstack() {
    return get_page(1);
    //return (void*)pt;
}

/**
 * @brief Free the memory that allocate by alloc_kstack
 * 
 * @param pid process id (start at 2)
 */
void free_kstack(void* pt) {
    free_page(pt, 1);
    //uint32_t pt = PAGE_SIZE_4MB * (KERNEL_INDEX + 1) - PAGE_SIZE * 2 * (pid + 2);
    //memset((char*)pt, 0, PAGE_SIZE *2);
}


// NOT IMPLEMENTED YET

// /**
//  * @brief Copies a block of arbitrary size from user space.
//  * 
//  * Side effect: The pointer from is an untrusted pointer 
//  * (a pointer that may be invalid in the current virtual address mapping)
//  * 
//  * @param to : Target address from user space
//  * @param from : Source address from kernel space
//  * @param n : Number of bytes to transfer
//  * @return int32_t : number of bytes denote success, 0 or negative values denote an error condition
//  */
// int32_t copy_from_user(void *to, const void *from, uint32_t n) {
//     int errno;
//     process_t *process;

//     if ((errno = validate_addr(from)) < 0) 
//         return errno;
    
//     /* perform transfer of data */
//     GETPRO(process);
    
//     memcpy(to, from, n);
//     return 1;
// }


// /**
//  * @brief Copies a block of arbitrary size to user space.
//  * 
//  * Side effect: The pointer to is an untrusted pointer 
//  * (a pointer that may be invalid in the current virtual address mapping)
//  * 
//  * @param to : Target address from kernel space
//  * @param from : Source address from user space
//  * @param n : Number of bytes to transfer
//  * @return int32_t : number of bytes denote success, 0 or negative values denote an error condition
//  */
// int32_t copy_to_user(void *to, const void *from, uint32_t n) {
//     int errno;
//     process_t *process;

//     if ((errno = validate_addr(to)) < 0) 
//         return errno;
    
//     /* TODO: perform transfer of data */
//     GETPRO(process);
    
//     memcpy(to, from, n);
//     return 1;
// }


// /**
//  * @brief Validate the input address inside the process address space
//  * 
//  * @param addr : A pointer points to the space to check
//  * @return int32_t : positive or 0 denote success, negative values denote an error condition
//  */
// static int32_t validate_addr(void *addr) {
//    /* There are two possible ways to perform this check: 
//     * (Perfer the 2nd one if you can! The first type of checking is quite time
//     * consuming because it must be executed for each address parameter included
//     * in a system call)
//     * 
//     * (1). Verify that the linear address belongs to the process address space
//     * and, if so, that the memory region including it has the proper access rights.
//     * 
//     * (2). Verify just that the linear address is lower than PAGE_OFFSET (i.e., that
//     * it does not fall within the range of interval address reserved to the kernel).
//     * 
//     */

//    /* Implmentation */
//    uint32_t a = (uint32_t) addr;
//    uint32_t size = (uint32_t) MAXADDR;

//    if (a + size < a)
//     //    a + size > curr_process->addr.limit.seg)
//        return 0;
       
//    return -EFAULT;
//     return 0;
// }
