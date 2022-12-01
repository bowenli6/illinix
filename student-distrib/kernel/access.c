#include <access.h>
#include <pro/process.h>
#include <errno.h>
#include <boot/x86_desc.h>
#include <boot/page.h>
#include <lib.h>
#include <kmalloc.h>
#include <io.h>



int _user_mem_mmap(thread_t* t) {
    int i, rtn = 0;
    for(i = 0; i < t->vm.size / PAGE_SIZE; i++) {
        rtn += mmap(USER_MEM + i * PAGE_SIZE, ADDR_TO_PTE(t->vm.mmap[i]), PAGE_SIZE, GETBIT_12(t->vm.mmap[i]));
    }  
    return rtn;
}

/**
 * @brief map user virtual memory to process pid's physical memory
 * 
 * @param pid process id (start at 2)
 */
void user_mem_map(thread_t* t) {
    int rtn = 0;
    if(t->vm.size == 0) {
        t->vm.size = PAGE_SIZE_4MB;
        rtn = vmalloc(&t->vm, 0, t->vm.size, PTE_RW | PTE_US);
        // if(rtn == 0) printf(">>>>>map process pid = %d succeed!\n", t->pid);
        // else printf(">>>>>map process pid = %d FAILED, vmalloc error\n", t->pid);
    }
    else {
        rtn = _user_mem_mmap(t);
        // if(rtn == 0) printf(">>>>>map process pid = %d succeed!\n", t->pid);
        // else printf(">>>>>map process pid = %d FAILED, mapping has existed\n", t->pid);
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
    int rtn;
    rtn = freemap(USER_MEM, t->vm.size);
    flush_tlb();
//     if(t->vm.size == 0){
//         printf("<<<<<unmap process pid = %d FAILED: empty map!\n", t->pid);
//     }
//     if(rtn == 0) printf("<<<<<unmap process pid = %d succeed!\n", t->pid);
//     else printf("<<<<<unmap process pid = %d FAILED!\n", t->pid);
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

