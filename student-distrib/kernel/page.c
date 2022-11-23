#include <boot/x86_desc.h>
#include <boot/page.h>
#include <kmalloc.h>
#include <lib.h>

/**
 * @brief Turn on paging related registers.
 *
 */

#define VIR_MEM_BEGIN           0x08000000      /* The mem begins at 128MB */

//static int pde_alloc_index = 2;
//pd_descriptor_t pdd[ENTRY_NUM];

pte_t* _walk(pagedir_t pd, uint32_t va, int alloc);

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
    kmalloc_init();
    /* initialize first 4MB directory */
    page_directory[0] = page_directory[0] | PTE_PRESENT | PTE_RW | ADDR_TO_PTE((int)page_table);

    /* initialize 4MB-8MB directory */
    page_directory[1] = page_directory[1] | PTE_PRESENT | PTE_RW | PDE_MB | PTE_GLO | (1 << PDE_OFFSET_4MB);
    //page_directory[2] = page_directory[2] | PTE_PRESENT | PTE_RW | PDE_MB | PTE_GLO | (1023 << PDE_OFFSET_4MB);

    /* initialize 8MB-4GB page directories */

    for(i = 2; i < 16; i++) {
        page_directory[i] = PTE_PRESENT | PTE_RW | PDE_MB | PTE_GLO | (i << PDE_OFFSET_4MB);
    }

    for(i = 16; i < ENTRY_NUM; i++){
        page_directory[i] = 0;    /* do not exist */
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





/*
pte_t* 
_walk(pagedir_t pd, uint32_t va, int alloc)
{
    pde_t* pde = &pd[va >> 22];
    int tempaddr;
    if(!((*pde) & PTE_PRESENT)) {
        if(!alloc) 
            return 0;
        else {
            if(tempaddr = (uint32_t)page_alloc() == 0) 
                return 0;
            *pde |= PTE_PRESENT | ADDR_TO_PTE(tempaddr);
        }
    }

    pagetable_t pagetable = PTE_ADDR(*pde) << 12;

    return &pagetable[ (va >> 12) & 0x3FF ];
}

int 
_mmap(pagedir_t pd, uint32_t va, uint32_t pa, int size, int flags)
{
    pte_t* pte;
    int i, walk_addr, length;

    walk_addr = va & 0xFFFFF000;
    length = ( size / PAGE_SIZE ) + 1;

    for( i = 0; i < length; i++ ){

        if(pte = _walk(pd, walk_addr, 1) == 0) 
            return -1;
        if((*pte) & PTE_PRESENT) 
            panic("remmap!"); 

        pte[0] = pte[0] | ( ADDR_TO_PTE(pa) + i * PAGE_SIZE ) | flags;
        walk_addr += PAGE_SIZE;
    }
    
    return 0;
}

int
_freemap(pagedir_t pd, uint32_t va, int size)
{
    pte_t* pte;
    uint32_t a;

    for(a = va; a <= va + size; a++) {

        if(pte = _walk(pd, a, 0) == 0) 
            panic("free a empty pointer");
        if(!((*pte) & PTE_PRESENT))
            panic("free invalid memory");

        kfree(PTE_ADDR(*pte) << 12);
        *pte = 0;
    }
}

void 
kmmap(uint32_t va, uint32_t pa, int size, int flags)
{
    if(_mmap(page_directory, va, pa, size, flags) != 0) 
        panic("kernel mmap error");
}


pagedir_t 
create_uvmdir()
{
    pagedir_t npd;
    if( npd = (pagedir_t) page_alloc() == 0 ) 
        return 0;
    memset(npd, 0, PAGE_SIZE);
    return npd;
}


int 
vmalloc(pagedir_t pd, int oldsize, int newsize, int flags)
{
    uint32_t startva, endva, va, pa;

    startva = ADDR_TO_PTE(oldsize) + PAGE_SIZE;
    endva = ADDR_TO_PTE(newsize);

    for(va = startva; va <= endva; va += PAGE_SIZE) {
        pa = page_alloc();
        if(pa == 0) 
            return -1;
        memset(pa, 0, PAGE_SIZE);
        if(_mmap(pd, va, pa, PAGE_SIZE, flags | PTE_RW | PTE_US) == -1) 
            return -1; 
    }

    return 0;
}


void 
vmdealloc(pagedir_t pd, int oldsize, int newsize)
{
    uint32_t startva, endva, va, pa;
    startva = ADDR_TO_PTE(newsize) + PAGE_SIZE;
    endva = ADDR_TO_PTE(oldsize);

    _freemap(pd, va, (startva - endva) / PAGE_SIZE);

    return;
}


void free_uvmdir(pagedir_t pd, int size)
{
    int i;
    _freemap(pd, 0, size / PAGE_SIZE);
    for(i = 0; i < ENTRY_NUM; i++) {
        if((pd[i] & PTE_PRESENT) && (pd[i] & PTE_US)) {
            kfree(ADDR_TO_PTE(pd[i]));
        }
    }
    kfree(pd);
    return;
}
*/