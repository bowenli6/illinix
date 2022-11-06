#include <boot/x86_desc.h>
#include <boot/page.h>

#define USER_MEM 0x8000000

void 
user_mem_map(int pid)
{
    page_directory[USER_MEM >> PDE_OFFSET_4MB] |= PTE_PRESENT | PTE_RW
         | PTE_US | PDE_MB | INDEX_TO_DIR(pid + 2);
    flush_tlb();
}