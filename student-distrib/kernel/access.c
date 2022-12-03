#include <access.h>
#include <pro/process.h>
#include <errno.h>
#include <boot/x86_desc.h>
#include <boot/page.h>
#include <lib.h>
#include <kmalloc.h>
#include <io.h>



int _user_mem_mmap(vm_area_t* vm) {
    int va, rtn = 0, i = 0;
    for(va = vm->vmstart; va < vm->vmend; va += PAGE_SIZE) {
        rtn += mmap(va, ADDR_TO_PTE(vm->mmap[i]), PAGE_SIZE, GETBIT_12(vm->mmap[i]));
        i ++;
    }  
    return rtn;
}

/**
 * @brief map user virtual memory to process pid's physical memory
 * 
 * @param pid process id (start at 2)
 */
void user_mem_map(thread_t* t) {
    int rtn = 0, temp;
    vm_area_t* area;
    if(t->vm.size == 0) {
        t->vm.size = 1;
        //rtn = vmalloc(t->vm.map_list, PAGE_SIZE_4MB, PTE_RW | PTE_US);

        area = t->vm.map_list;
        while(area != 0) {
            // if(area->vmflag & VM_STACK) {
            //    area->vmstart = area->vmend = USER_STACK_ADDR - PAGE_SIZE * 64;
            //    rtn += vmalloc(area, PAGE_SIZE * 64, PTE_RW * ((area->vmflag & VM_WRITE)? 1: 0) | PTE_US);
            // }
            temp = area->vmend - area->vmstart;
            area->vmend = area->vmstart;
            rtn += vmalloc(area, temp, PTE_RW * ((area->vmflag & VM_WRITE)? 1: 0) | PTE_US);
            area = area->next;
        }
        
        // if(rtn == 0) printf(">>>>>map process pid = %d succeed!\n", t->pid);
        // else printf(">>>>>map process pid = %d FAILED, vmalloc error\n", t->pid);
    }
    else {
        area = t->vm.map_list;
        while(area != 0) {
            rtn += _user_mem_mmap(area);
            area = area->next;
        }
        
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
    vm_area_t *area = t->vm.map_list;
    while(area != 0){
        rtn = freemap(area->vmstart, area->vmend - area->vmstart);
        area = area->next;
    }
    
    flush_tlb();

    // if(rtn == 0) printf("<<<<<unmap process pid = %d succeed!\n", t->pid);
    // else printf("<<<<<unmap process pid = %d FAILED!\n", t->pid);
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

