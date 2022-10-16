#include "../lib/lib.h"
#include "../x86_desc.h"
#include "../include/page.h"

/**
 * @brief Turn on paging related registers.
 *
 */
void enable_paging()
{
    /* set CR3 to directory base address */
    asm volatile(
	"movl %0, %%eax             ;"
	"movl %%eax, %%cr3          ;"
	:  : "r"(page_directory): "eax" );

    /* Turn on page size extension */
    asm volatile(
    "movl %%cr4, %%eax          ;"
    "orl %0, %%eax           ;"
    "movl %%eax, %%cr4          ;"
	:  : "r"(CR4_EXTENSION_FLAG): "eax" );

    /* Turn on paging */
    asm volatile(
	"movl %%cr0, %%eax          ;"
	"orl %0, %%eax     ;"
	"movl %%eax, %%cr0          ;"
	:  : "r"(CR0_PAGE_FLAG): "eax" );
}

/**
 * @brief Initialize page directory and page table.
 * 
 */
void page_init()
{
    int i;

    /* initialize first 4MB directory */
    page_directory[0].KB.present = 1;
    page_directory[0].KB.read_write = 1;
    page_directory[0].KB.base_address = (int)page_table >> PDE_OFFSET_4KB;

    /* initialize 4MB-8MB directory */
    page_directory[1].MB.present = 1;
    page_directory[1].MB.read_write = 1;
    page_directory[1].MB.page_size = 1;
    page_directory[1].MB.global_page = 1;
    page_directory[1].MB.base_address = 1;

    /* initialize 8MB-4GB page directories */
    for(i = 2; i < ENTRY_NUM; i++){
        page_directory[i].MB.present = 0;   /* do not exist */
        page_directory[i].MB.page_size = 1;
        page_directory[i].MB.base_address = i;
    }
    
    /* initialize page tables */
    for(i = 0; i < ENTRY_NUM; i++)
    {
        /* only video memory is initialized as present */
        page_table[i].present = (i == ( VIDEO >> PDE_OFFSET_4KB ) ) ? 1 : 0;
        page_table[i].read_write = 1;
        page_table[i].base_address = (i == ( VIDEO >> PDE_OFFSET_4KB ) ) ? i : 0;
    }

    /* turn on paging registers */
    enable_paging();

    return;
}

