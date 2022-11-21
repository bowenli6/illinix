#include <boot/x86_desc.h>
#include <boot/page.h>
#include <lib.h>

#define VIR_MEM_BEGIN           0x08000000      /* The mem begins at 128MB */

//static int pde_alloc_index = 2;
//pd_descriptor_t pdd[ENTRY_NUM];

pte_t* _walk(pagedir_t pd, uint32_t va, int alloc);


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

void flush_tlb()
{
    asm volatile(
    "movl %%cr3, %%eax      ;"
    "movl %%eax, %%cr3      ;"
    : : : "eax" );
}

/**
 * @brief Initialize page directory and page table.
 * 
 */
void page_init()
{
    int i;

    /* initialize first 4MB directory */
    page_directory[0] = page_directory[0] | PTE_PRESENT | PTE_RW | ADDR_TO_PTE((int)page_table);

    /* initialize 4MB-8MB directory */
    page_directory[1] = page_directory[1] | PTE_PRESENT | PTE_RW | PDE_MB | PTE_GLO | (1 << PDE_OFFSET_4MB);

    /* initialize 8MB-4GB page directories */
    for(i = 2; i < ENTRY_NUM; i++){
        page_directory[i] = page_directory[i] | PDE_MB;    /* do not exist */
    }
    
    /* initialize page tables */
    for(i = 0; i < ENTRY_NUM; i++)
    {
        /* only video memory is initialized as present */
        if(i == (VIDEO >> PDE_OFFSET_4KB) ) {
            page_table[i] = page_table[i] | PTE_PRESENT | PTE_RW | ADDR_TO_PTE(VIDEO); 
        }
        else {
            page_table[i] = 0 | PTE_RW;  
        }
    }

    page_directory[VIR_VID_MEM / PAGE_SIZE_4MB] = PTE_PRESENT | PTE_RW | PTE_US | ADDR_TO_PTE((int)vidmap_table);
    for(i = 0; i < ENTRY_NUM; i++) {
        if(i == (VIDEO >> PDE_OFFSET_4KB)) {
            vidmap_table[i] = PTE_PRESENT | PTE_RW | PTE_US | ADDR_TO_PTE(VIDEO);
        }
        else {
            vidmap_table[i] = 0;
        }
    }

    /* turn on paging registers */
    enable_paging();
    return;
}



/**
 * @brief map the text-mode video memory into user space at a pre-set virtual address.
 * 
 * @param screen_start : starting screen address
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
int32_t do_vidmap(uint8_t **screen_start) {
    if(((uint32_t)screen_start) < VIR_MEM_BEGIN || ((uint32_t)screen_start) > (VIR_MEM_BEGIN + PAGE_SIZE_4MB)) {
        return -1;
    }
    *screen_start = (uint8_t*) (VIR_VID_MEM + VIDEO);
    
    return 0;
}


