#include <boot/x86_desc.h>
#include <boot/page.h>
#include <lib.h>
#include <kmalloc.h>

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

pte_t* walk(uint32_t va, int alloc)
{
    pde_4KB_t* pde = &page_directory[va >> 22].KB;
    if(!pde->available ) {
        if(!alloc) 
            return 0;
        else {
            if(pde->base_address = ( ( (uint32_t)page_alloc() ) >> 12 ) == 0) 
                return 0;
            pde->available = 1;
        }
    }

    pagetable_t pagetable = pde->base_address << 12;

    return &pagetable[ va >> 12 ];
}

int _mmap(uint32_t va, uint32_t pa, int size, int rw, int us)
{
    pte_t* pte;
    int i, walk_addr, length;

    walk_addr = va & 0xFFFFF000;
    length = ( size / PAGE_SIZE ) + 1;

    for( i = 0; i < length; i++ ){

        if(pte = walk(walk_addr , 1) == 0) 
            return -1;
        if(pte->available) panic("remmap!"); 

        pte->base_address = ( pa >> 12 ) + i;
        pte->available = 1;
        pte->read_write = rw;
        pte->user_supervisor = us;
        walk_addr += PAGE_SIZE;
    }
    
    return 0;
}

void kmmap(uint32_t va, uint32_t pa, int size, int rw, int us)
{
    if(_mmap(va, pa, size, rw, us) != 0) panic("kernel mmap error");
}

