#include <boot/x86_desc.h>
#include <boot/page.h>

#define USER_MEM 0x8000000

/**
 * @brief map user virtual memory to process pid's physical memory
 * 
 * @param pid process id
 */
extern void 
user_mem_map(int pid)
{
    page_directory[USER_MEM >> PDE_OFFSET_4MB] |= PTE_PRESENT | PTE_RW
         | PTE_US | PDE_MB | INDEX_TO_DIR(pid + 2);
    flush_tlb();
}

/**
 * @brief alloc a 8KB memory in kernel for process pid
 * 
 * @param pid process id
 * @return void* pointer to the process kernel stack
 */
extern void*
alloc_kstack(int pid)
{
    uint32_t pt = PAGE_SIZE_4MB * (KERNEL_INDEX + 1) - PAGE_SIZE * 2 * (pid + 1);
    return (void*)pt;
}