#include <access.h>
#include <pro/process.h>
#include <errno.h>
#include <boot/x86_desc.h>
#include <boot/page.h>
#include <lib.h>


/**
 * @brief map user virtual memory to process pid's physical memory
 * 
 * @param pid process id (start at 2)
 */
void user_mem_map(pid_t pid) {
    page_directory[VIR_MEM_BEGIN >> PDE_OFFSET_4MB] |= PTE_PRESENT | PTE_RW
         | PTE_US | PDE_MB | INDEX_TO_DIR(pid);
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
void *alloc_kstack(pid_t pid) {
    uint32_t pt = PAGE_SIZE_4MB * (KERNEL_INDEX + 1) - PAGE_SIZE * 2 * (pid + 2);
    return (void*)pt;
}

/**
 * @brief Free the memory that allocate by alloc_kstack
 * 
 * @param pid process id (start at 2)
 */
void free_kstack(pid_t pid) {
    uint32_t pt = PAGE_SIZE_4MB * (KERNEL_INDEX + 1) - PAGE_SIZE * 2 * (pid + 2);
    memset((char*)pt, 0, PAGE_SIZE *2);
}
