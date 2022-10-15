#include "lib.h"
#include "x86_desc.h"
#include "page.h"

void page_init()
{
    int i;
    /* initialize first 4MB directory */
    page_directory[0].KB.present = 1;
    page_directory[0].KB.read_write = 1;
    page_directory[0].KB.user_supervisor = 0;
    page_directory[0].KB.write_through = 0;
    page_directory[0].KB.cache_disabled = 0;
    page_directory[0].KB.accessed = 0;
    page_directory[0].KB.reserved = 0;
    page_directory[0].KB.page_size = 0;
    page_directory[0].KB.global_page = 0;
    page_directory[0].KB.available = 0;
    page_directory[0].KB.base_address = (int)page_table >> PDE_OFFSET_4KB;

    /* initialize 4MB-8MB directory */
    page_directory[1].MB.present = 1;
    page_directory[1].MB.read_write = 1;
    page_directory[1].MB.user_supervisor = 0;
    page_directory[1].MB.write_through = 0;
    page_directory[1].MB.cache_disabled = 0;
    page_directory[1].MB.accessed = 0;
    page_directory[1].MB.dirty = 0;
    page_directory[1].MB.page_size = 1;
    page_directory[1].MB.global_page = 1;
    page_directory[1].MB.available = 3;
    page_directory[1].MB.pat_flag = 0;
    page_directory[1].MB.reserved = 0;
    page_directory[1].MB.base_address = 1;

    /* initialize 8MB-4GB page directories */
    for(i = 2; i < ENTRY_NUM; i++){
        page_directory[i].MB.present = 0;
        page_directory[i].MB.read_write = 0;
        page_directory[i].MB.user_supervisor = 0;
        page_directory[i].MB.write_through = 0;
        page_directory[i].MB.cache_disabled = 0;
        page_directory[i].MB.accessed = 0;
        page_directory[i].MB.dirty = 0;
        page_directory[i].MB.page_size = 1;
        page_directory[i].MB.global_page = 0;
        page_directory[i].MB.available = 0;
        page_directory[i].MB.pat_flag = 0;
        page_directory[i].MB.reserved = 0;
        page_directory[i].MB.base_address = i;
    }
    
    /* initialize page tables */
    for(i = 0; i < ENTRY_NUM; i++)
    {
        page_table[i].present = (i == VIDEO) ? 1 : 0;
        page_table[i].read_write = 1;
        page_table[i].user_supervisor = 0;
        page_table[i].write_through = 0;
        page_table[i].cache_disabled = 0;
        page_table[i].accessed = 0;
        page_table[i].dirty = 0;
        page_table[i].pat_flag = 0;
        page_table[i].global_page = 0;
        page_table[i].available = 0;
        page_table[i].base_address = i;
    }

    return;
}

/* Turn on page size extension; set directory address; enable paging */
void enable_paging()
{
    asm volatile(
    "movl %%cr4, %%eax          ;"
    "orl $0x10, %%eax           ;"
    "movl %%eax, %%cr4          ;"
	"movl %0, %%eax             ;"
	"movl %%eax, %%cr3          ;"
	"movl %%cr0, %%eax          ;"
	"orl $0x80000000, %%eax     ;"
	"movl %%eax, %%cr0          ;"
	:  : "r"(page_directory): "eax" );
}
