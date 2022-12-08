#include <boot/x86_desc.h>
#include <boot/page.h>
#include <kmalloc.h>
#include <lib.h>
#include <pro/process.h>
#include <io.h>

/**
 * @brief Turn on paging related registers.
 *
 */

#define VIR_MEM_BEGIN           0x08000000      /* The mem begins at 128MB */

//static int pde_alloc_index = 2;
//pd_descriptor_t pdd[ENTRY_NUM];

//int vmalloc(vmem_t* vm, uint32_t start_addr, int oldsize, int newsize, int flags);
pte_t* _walk(uint32_t va, uint32_t flag, int alloc);
buddy* get_buddy(uint32_t addr);

user_page_t u1, u2;
user_page_t* upage_4mb;
user_page_t* upage_4kb;

free_area_t ufree_area[MAX_ORDER +1];
buddy ufree_list[MAX_ORDER + 1];

pd_descriptor_t pdesc[ENTRY_NUM];

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

    /* Initialize page directory descriptors */
    for(i = 0; i < ENTRY_NUM; i++) {
        pdesc->flags = 0;
        pdesc->count = 0;
    }
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
        page_table[VIDEO_BUF_1 >> PDE_OFFSET_4KB] = PTE_PRESENT | PTE_RW | ADDR_TO_PTE(VIDEO_BUF_1); 
        page_table[VIDEO_BUF_2 >> PDE_OFFSET_4KB] = PTE_PRESENT | PTE_RW | ADDR_TO_PTE(VIDEO_BUF_2);
        page_table[VIDEO_BUF_3 >> PDE_OFFSET_4KB] = PTE_PRESENT | PTE_RW | ADDR_TO_PTE(VIDEO_BUF_3);

    /* turn on paging registers */
    enable_paging();
    return;
}



int32_t do_vidmap(uint8_t **screen_start)
{
    // cli();
    if(!(*(_walk((uint32_t)screen_start, 0, 0)) & PTE_US)) {
        return -1;
    }

    thread_t* t;
    GETPRO(t);

    *screen_start = current->vidmap;

    // if(t == current->task) {
    //     *screen_start = (uint8_t*) (VIR_VID_MEM + VIDEO);
    // }
    // else {
    //     *screen_start = (uint8_t*) (t->terminal->saved_vidmem);
    // }
    // sti();
    return 0;
}




/**
 * @brief Get the buddy object
 * 
 * @param addr Addresses to be allocated
 * @return buddy* buddy struct storing addr
 */
buddy* get_buddy(uint32_t addr) 
{
    buddy* b = kmalloc(sizeof(buddy));
    b->addr = addr;
    return b;
}


/**
 * @brief Initialize user memory area
 *        (bitmaps & free lists), 
 *        index user memory
 */
void user_mem_init() 
{
    int i = 0;
    buddy* p;

    /* create space for bitmaps */
    ufree_area[MAX_ORDER].bmsize = 0;
    for(i = 0; i < MAX_ORDER; i++) {
        ufree_area[i].bmsize = ((PAGE_SIZE_4MB * 2) / buddy_size(i) + 31) / 32;
        ufree_area[i].bit_map = kmalloc(ufree_area[i].bmsize * sizeof(uint32_t));
    }

    /* Initialize user free lists */
    for(i = 0; i <= MAX_ORDER; i++) {
        buddy* fl = &ufree_list[i];
        ufree_area[i].free_list = fl;
        fl->last = fl;
        fl->next = fl; 
        memset(ufree_area[i].bit_map, 0, ufree_area[i].bmsize * sizeof(uint32_t)); /* All buddys are both free */
        ufree_area[i].start_addr = KERNEL_PAGES * PAGE_SIZE_4MB;
        ufree_area[i].user = 1;
    }

    buddy* fl = ufree_area[MAX_ORDER].free_list;
    /* index user memory and store in largest free list */
    for(i = KERNEL_PAGES * PAGE_SIZE_4MB; i < MAX_PHYS_PAGES * PAGE_SIZE_4MB; i += buddy_size(MAX_ORDER - 1)) {
        p = get_buddy(i);
        free_list_push(fl, p);
    }
}



/**
 * @brief   Allocate 2^order page size user memory space.
 *          Using the buddy allocator.
 * 
 * @param order     Required memory space, can be any integer from 0 to 4MB.
 * @return void*    pointer to allocated memory space. NULL means failed.
 */
uint32_t get_user_page(int order)
{
    int i, rtn;
    buddy* temp;
    if(order >= MAX_ORDER || order < 0) return NULL;

    for(i = order; i <= MAX_ORDER; i++) {
        /* find the smallest order that has a free block */ 
        if((temp = free_list_pop(ufree_area[i].free_list)) == NULL) {
            continue;
        }
        temp = buddy_split(ufree_area, temp, i, order); /* reduce order with split */
        rtn = temp->addr;
        kfree(temp);
        return rtn;
    }

    return NULL;
}

/**
 * @brief       Free a memory address created by get_user_page().
 * 
 * @param addr  Physical address of allocated space.
 * @param order The minimum order is 0 (4KB) and the maximum order is 10 (4MB).
 */
void free_user_page(uint32_t addr, int order)
{   
    _free_page(ufree_area, get_buddy(addr), order);
    return;
}


/**
 * @brief   Find the page table entry corresponding to a virtual address.
 * 
 * @param va        target virtual address
 * @param flags     Used to create page directory entry
 * @param alloc     If the value of alloc is 1, _walk will create a page table
 *                  for a pde that does not currently exist; otherwise, _walk
 *                  will return 0 if the pde does not exist.
 * @return pte_t*   pointer to target pte
 */
pte_t*
_walk(uint32_t va, uint32_t flags, int alloc)
{
    uint32_t pde_i = PDE_MB_ADDR(va);
    pde_t* pde = &page_directory[pde_i];                        /* Get pde entry. */
    uint32_t ptaddr;

    if(!(*pde & PTE_PRESENT)) {
        if(!alloc || (ptaddr = (uint32_t)get_page(0)) == 0)     /* Alloc a new page table if needed. */
            return 0;
        memset((void*)ptaddr, 0, PAGE_SIZE);
        *pde = PTE_PRESENT | flags | ADDR_TO_PTE(ptaddr);
    }

    pte_t* pte = (pte_t*) ADDR_TO_PTE(*pde);

    return &pte[(va >> VA_OFFSET) & GETBIT_10];
}

/**
 * @brief           Create a memory map between a virtual address and 
 *                  a physical address.
 * 
 * @param va        Virtual address.
 * @param pa        Physical address.
 * @param size      Size of memory map. (# bytes)
 * @param flags     Flags to be set on PTE & PDE.
 * @return int      0 if succeed, -1 if failed.
 */
int mmap(uint32_t va, uint32_t pa, int size, int flags)
{
    pte_t* pte;
    int i, addr, length;
    
    addr = ADDR_TO_PTE(va);
    length = size / PAGE_SIZE;

    for(i = 0; i < length; i++ ){

        if((pte = _walk(addr, flags, 1)) == 0)          /* Get the PTE position to map. */
            panic("walk error");

        if((*pte) & PTE_PRESENT)                        /* Cannot remap an existing mmap. */
            return -1;

        *pte = PTE_PRESENT | flags | (ADDR_TO_PTE(pa) + i * PAGE_SIZE);     /* Create map. */
        pdesc[PDE_MB_ADDR(addr)].count ++;

        addr += PAGE_SIZE;
    }

    flush_tlb();
    return 0;
}

/**
 * @brief           Delete a memory map on a virtual address.
 *                  *Do not free the physical memory.*
 * 
 * @param va        Virtual address.
 * @param size      Size of map to delete.
 * @return int      0 if succeed, -1 if failed.
 */
int
freemap(uint32_t va, int size)
{
    pte_t* pte;
    uint32_t addr;

    for(addr = va; addr < va + size; addr += PAGE_SIZE) {
        if((pte = _walk(addr, 0, 0)) == 0)              /* Find the PTE to free. */
            return -1;

        if(!((*pte) & PTE_PRESENT))
            return -1;

        *pte = 0;
        if((pdesc[PDE_MB_ADDR(addr)].count--) == 0) {   /* If the page table became empty, free it. */
            free_page((void*)ADDR_TO_PTE(page_directory[PDE_MB_ADDR(addr)]), 0);
        }
    }

    flush_tlb();
    return 0;
}


/**
 * @brief       Initialize the virtual memory structure of a process.
 *              * Do not alloc physical memory *
 * 
 * @param vm    Virtual memory struct.
 */
void process_vm_init(vmem_t* vm)
{
    vm_area_t *file = kmalloc(sizeof(vm_area_t));
    vm_area_t *heap = kmalloc(sizeof(vm_area_t));
    vm_area_t *stack = kmalloc(sizeof(vm_area_t));

    vm->size = 0;
    vm->file_length = 0;
    vm->start_brk = vm->brk = 0x8800000;
    vm->count = 0;

    file->next = heap;
    file->vmend = PROGRAM_IMG_BEGIN + PAGE_SIZE * vm->file_length;
    file->vmstart = PROGRAM_IMG_BEGIN;
    // file->vmend = USER_MEM + PAGE_SIZE_4MB;
    file->vmflag = VM_READ | VM_WRITE | VM_EXEC;
    
    heap->vmstart = heap->vmend = vm->brk;
    heap->vmflag = VM_READ | VM_WRITE | VM_HEAP;
    heap->next = stack;

    stack->vmend = 0xC000000;
    stack->vmstart = stack->vmend;
    stack->next = 0;
    stack->vmflag = VM_READ | VM_WRITE | VM_STACK;

    vm->map_list = file;
    
}


/**
 * @brief       Expand a virtual memory area. Allocate physical memory 
 *              to it and create the mapping.
 * 
 * @param vm        Virtual memory area.
 * @param incrsize  Increase size.
 * @param flags     Flags of memory map.
 * @return int      0 if succeed, -1 if failed.
 */
int 
vmalloc(vm_area_t* vm, int incrsize, int flags)
{
    uint32_t startva, va, pa;
    int i = 0, length, incrlength;
    uint32_t* temp;

    if(incrsize < 0) 
        return -1;
    if(incrsize == 0)
        return 0;

    incrlength = (incrsize + PAGE_SIZE - 1) / PAGE_SIZE;

    length = (vm->vmend - vm->vmstart) / PAGE_SIZE;

    if((temp = kmalloc((length + incrlength) * sizeof(uint32_t*))) == 0)    /* Create space for mmap area structure. */
        return -1;
    if(length) {
        memcpy(temp, vm->mmap, length * sizeof(uint32_t*));                 /* If it has an old mmap area, replace it. */
        kfree(vm->mmap);
    }
    vm->mmap = temp;

    startva = vm->vmend;
    
    vm->vmend = vm->vmend + ADDR_TO_PTE(incrsize + PAGE_SIZE - 1);

    for(va = startva; va < vm->vmend; va += PAGE_SIZE) {
        if((pa = get_user_page(0)) == 0)                                    /* Alloc physical memory. */
            return -1;
        if(mmap(va, pa, PAGE_SIZE, flags) == -1)                            /* Map it onto the virtual address. */
            panic("mmap error");
        vm->mmap[i] = PTE_PRESENT | flags | (ADDR_TO_PTE(pa));              /* Store the mmap info into the process's structure. */
        i++;
    }

    flush_tlb();
    return 0;
}

/**
 * @brief           Dealloc a virtual memory area. Dealloc the physical memory corresponding
 *                  to it. If it's mapped on current virtual memory, delete the memory map.
 * 
 * @param vm        Virtual memory area.
 * @param decsize   Decrease size.
 * @param mapping   1 if mapping on virtual memory, 0 if not mapping.
 */
void vmdealloc(vm_area_t* vm, int decsize, int mapping)
{
    uint32_t newend, va;
    int i, pa;
    uint32_t* temp;
    
    i = (vm->vmend - vm->vmstart) / PAGE_SIZE;

    newend = vm->vmend - decsize / PAGE_SIZE;

    /* Loop through all the virtual addresses need to be freed. */
    for(va = vm->vmend; va > newend; va -= PAGE_SIZE) {

        if(mapping)                         /* If the address if mapped on vm, delete it. */
            freemap(va, PAGE_SIZE);     
        
        pa = ADDR_TO_PTE(vm->mmap[--i]);    /* Fetch physical address from mmap structure. */
        free_user_page(pa, 0);              /* Free the physical address. */
    }

    if(newend != vm->vmend) {               /* Shrink the mmap structure. */
        temp = kmalloc(i * sizeof(uint32_t*));          
        memcpy(temp, vm->mmap, i * sizeof(uint32_t*));
        kfree(vm->mmap);   

        vm->mmap = temp;
        vm->vmend = newend; 
    }
    
}

/**
 * @brief           Copy a virtual memory structure.
 * 
 * @param dest      Destination vm structure.
 * @param src       Source vm structure.
 * @return int      0 if succeed, -1 if failed.
 */
int vmcopy(vmem_t* dest, vmem_t* src) 
{
    uint32_t i, length, pa, va, cachepa;
    pte_t* pte;
    char *cache;
    vm_area_t* srcarea, *destarea, *nextarea;

    /* Use a temp storage. It's a random address in user area. */
    cachepa = get_user_page(0);
    cache = (char*) 0x7F00000;
    mmap((uint32_t)cache, cachepa, PAGE_SIZE, PTE_US | PTE_RW);


    dest->size = src->size;                     /* Copy virtual memory attributes. */
    dest->start_brk = src->start_brk;
    dest->file_length = src->file_length;
    dest->brk = src->brk;
    //dest->count = ++src->count;

    srcarea = src->map_list;                    /* Start from the first memory map area. */

    destarea = dest->map_list;
    while(destarea != 0) {
        nextarea = destarea->next;
        kfree(destarea);                        /* Free destination mmap areas if it has existed. */
        destarea = nextarea;
    }

    destarea = kmalloc(sizeof(vm_area_t));      /* Create first destination mmap area. */
    dest->map_list = destarea;

    while(srcarea != 0) {
        length = (srcarea->vmend - srcarea->vmstart) / PAGE_SIZE;

        destarea->mmap = kmalloc(sizeof(uint32_t*) * length);

        destarea->vmstart = srcarea->vmstart;
        destarea->vmend = srcarea->vmend;
        destarea->vmflag = srcarea->vmflag;

        /* For each mmap area to copy, loop through all pages. */
        i = 0;
        for(va = srcarea->vmstart; va < srcarea->vmend; va += PAGE_SIZE) {

            if((pte = _walk(va, 0, 0)) == 0) {      /* Obtain PTE entry for current virtual address. */
                panic("vmcopy: walk error");
            }
            if((*pte & PTE_PRESENT) == 0) {
                panic("vmcopy: src not present");
            }
            
            if((pa = get_user_page(0)) == 0) {      /* Alloc physical memory for dest. */
                panic("vmcopy: get user page failed");
            }
            
            memcpy(cache, (char*)va, PAGE_SIZE);    /* Copy mapping data onto the temp storage. */
            
            freemap(va, PAGE_SIZE);                 /* Delete previous memory map. */

            if(mmap(va, pa, PAGE_SIZE, GETBIT_12(srcarea->mmap[i])) == -1) {    /* Map new physical memory to the pagedir. */
                free_user_page(pa, 0);
                panic("vmcopy: remap failed");
            }
            
            memcpy((char*)va, cache, PAGE_SIZE);    /* Copy data to current virtual memory. */

            destarea->mmap[i] = PTE_PRESENT | GETBIT_12(srcarea->mmap[i]) | (ADDR_TO_PTE(pa));
            i ++;
        }

        srcarea = srcarea->next;
        if(srcarea != 0) {
            nextarea = kmalloc(sizeof(vm_area_t));  /* Create next mmap area structure. */
            destarea->next = nextarea;
            destarea = nextarea;
        }
    }

    destarea->next = 0;

    freemap((uint32_t)cache, PAGE_SIZE);            /* Free the temp storage. */
    free_user_page(cachepa, 0);

    return 0;
}

void show_mmap(vmem_t* vm)
{
    vm_area_t* area;
    area = vm->map_list;
    int length;
    printf("-----------------memory-map------------------\n");
    printf("start           end           length     flag\n");
    while(area->next != 0) {
        length = area->vmend - area->vmstart;
        if(length != 0) {
            printf("0x%x-----", area->vmstart);
            printf("0x%x      ", area->vmend);
            printf("0x%x      ",length);
            if(area->vmflag & VM_HEAP) {
                printf("[HEAP]\n");
                area = area->next;
                continue;
            }
            if(area->vmflag & VM_STACK) {
                printf("[STACK]\n");
                area = area->next;
            }
            printf("r%c%c\n", (area->vmflag & VM_WRITE)?'w':'-',(area->vmflag & VM_EXEC)?'e':'-');
        }
        area = area->next;
    }
    printf("-----------------memory-map------------------\n");
}

