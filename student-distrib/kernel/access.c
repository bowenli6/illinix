#include <access.h>
#include <pro/process.h>
#include <errno.h>
#include <boot/x86_desc.h>
#include <boot/page.h>
#include <lib.h>
#include <kmalloc.h>



void _user_mem_mmap(thread_t* t) {
    int i;
    for(i = 0; i < t->vm.size / PAGE_SIZE; i++) {
        mmap(USER_MEM + i * PAGE_SIZE, ADDR_TO_PTE(t->vm.mmap[i]), PAGE_SIZE, GETBIT_12(t->vm.mmap[i]));
    }  
}

/**
 * @brief map user virtual memory to process pid's physical memory
 * 
 * @param pid process id (start at 2)
 */
void user_mem_map(thread_t* t) {
    if(t->vm.size == 0) {
        t->vm.size = PAGE_SIZE_4MB;
        vmalloc(&t->vm, 0, t->vm.size, PTE_RW | PTE_US);
    }
    else {
          _user_mem_mmap(t);
    }
    flush_tlb();
}


// void user_mem_map(thread_t* t) {
//     page_directory[VIR_MEM_BEGIN >> PDE_OFFSET_4MB] = PTE_PRESENT | PTE_RW
//          | PTE_US | PDE_MB | INDEX_TO_DIR(t->pid + 14);
//     flush_tlb();
// }


// void user_mem_unmap(thread_t* t) {
//     page_directory[VIR_MEM_BEGIN >> PDE_OFFSET_4MB] = 0;
//     flush_tlb();
// }

/**
 * @brief user_mem_unmap
 * 
 * @param pid process id (start at 2)
 */
void user_mem_unmap(thread_t* t) {
    freemap(USER_MEM, t->vm.size);
    flush_tlb();
}

/**
 * @brief Alloc a 8KB memory in kernel for process pid
 * 
 * @param pid process id (start at 2)
 * @return void* pointer to the process kernel stack
 */
void *alloc_kstack(void) {
    return get_page(1);
}

/**
 * @brief Free the memory that allocate by alloc_kstack
 * 
 * @param pid process id (start at 2)
 */
void free_kstack(void* pt) {
    free_page(pt, 1);
}

