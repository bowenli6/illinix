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
    if(*(_walk((uint32_t)screen_start, 0, 0)) & PTE_US) {
        return -1;
    }
    *screen_start = (uint8_t*) (VIR_VID_MEM + VIDEO);
    
    return 0;
}

/**
 * @brief Expand the current thread's heap size.
 * 
 * @param incrsize The size(# bytes) of the heap expansion requested by the user. 
 */
void do_mmap(int incrsize) {
    thread_t* curr;
    vm_area_t *heap;
    
    GETPRO(curr);

    curr->vm.brk += (incrsize + PAGE_SIZE - 1) / PAGE_SIZE;

    heap = curr->vm.map_list;

    while(heap != 0) {
        if(heap->vmflag & VM_HEAP) {
            vmalloc(heap, incrsize, PTE_US | PTE_RW);
            return;
        }
        heap = heap->next;
    }

    panic("Where is my heap??");
}



buddy* get_buddy(uint32_t addr) 
{
    buddy* b = kmalloc(sizeof(buddy));
    b->addr = addr;
    return b;
}


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
 * @brief Get a free user page using buddy system
 * 
 * @param order 
 * @return void* 
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

void free_user_page(uint32_t addr, int order)
{   
    _free_page(ufree_area, get_buddy(addr), order);
    return;
}



pte_t*
_walk(uint32_t va, uint32_t flags, int alloc)
{
    uint32_t pde_i = PDE_MB_ADDR(va);
    pde_t* pde = &page_directory[pde_i];
    uint32_t ptaddr;

    if(!(*pde & PTE_PRESENT)) {
        if(!alloc || (ptaddr = (uint32_t)get_page(0)) == 0)
            return 0;
        memset((void*)ptaddr, 0, PAGE_SIZE);
        *pde = PTE_PRESENT | flags | ADDR_TO_PTE(ptaddr);
    }

    pte_t* pte = (pte_t*) ADDR_TO_PTE(*pde);

    return &pte[(va >> VA_OFFSET) & GETBIT_10];
}


int mmap(uint32_t va, uint32_t pa, int size, int flags)
{
    pte_t* pte;
    int i, addr, length;
    
    addr = ADDR_TO_PTE(va);
    length = size / PAGE_SIZE;

    for(i = 0; i < length; i++ ){
        if((pte = _walk(addr, flags, 1)) == 0) 
            panic("walk error");
        if((*pte) & PTE_PRESENT) 
            return -1;

        *pte = PTE_PRESENT | flags | (ADDR_TO_PTE(pa) + i * PAGE_SIZE);
        pdesc[PDE_MB_ADDR(addr)].count ++;

        addr += PAGE_SIZE;
    }
    flush_tlb();
    return 0;
}

int
freemap(uint32_t va, int size)
{
    pte_t* pte;
    uint32_t addr;

    for(addr = va; addr < va + size; addr += PAGE_SIZE) {
        if((pte = _walk(addr, 0, 0)) == 0) 
            return -1;
        if(!((*pte) & PTE_PRESENT))
            return -1;
        *pte = 0;
        if((pdesc[PDE_MB_ADDR(addr)].count--) == 0) {
            free_page((void*)ADDR_TO_PTE(page_directory[PDE_MB_ADDR(addr)]), 0);
        }
    }
    flush_tlb();
    return 0;
}



void process_vm_init(vmem_t* vm)
{
    vm_area_t *file = kmalloc(sizeof(vm_area_t));
    vm_area_t *heap = kmalloc(sizeof(vm_area_t));
    vm_area_t *stack = kmalloc(sizeof(vm_area_t));

    vm->size = 0;
    vm->file_length = 16;
    vm->start_brk = vm->brk = 0x8800000;
    vm->count = 0;

    file->next = heap;
    file->vmstart = PROGRAM_IMG_BEGIN;
    file->vmend = PROGRAM_IMG_BEGIN + PAGE_SIZE * vm->file_length;
    // file->vmstart = USER_MEM;
    // file->vmend = USER_MEM + PAGE_SIZE_4MB;
    file->vmflag = VM_READ | VM_WRITE | VM_EXEC;
    
    heap->vmstart = heap->vmend = vm->brk;
    heap->vmflag = VM_READ | VM_WRITE | VM_HEAP;
    heap->next = stack;

    stack->vmend = 0xC000000;
    stack->vmstart = stack->vmend - 16 * PAGE_SIZE;
    stack->next = 0;
    stack->vmflag = VM_READ | VM_WRITE | VM_STACK;

    vm->map_list = file;
    
}


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

    if((temp = kmalloc((length + incrlength) * sizeof(uint32_t*))) == 0)
        return -1;
    if(length) {
        memcpy(temp, vm->mmap, length * sizeof(uint32_t*));
        kfree(vm->mmap);
    }
    vm->mmap = temp;

    startva = vm->vmend;
    // startva = USER_MEM + ADDR_TO_PTE(oldsize) + PAGE_SIZE * ((oldsize % PAGE_SIZE) != 0);
    // endva =  ADDR_TO_PTE(USER_MEM + newsize + PAGE_SIZE - 1);
    
    vm->vmend = vm->vmend + ADDR_TO_PTE(incrsize + PAGE_SIZE - 1);

    for(va = startva; va < vm->vmend; va += PAGE_SIZE) {
        if((pa = get_user_page(0)) == 0)
            return -1;
        if(mmap(va, pa, PAGE_SIZE, flags) == -1) 
            panic("mmap error");
        vm->mmap[i] = PTE_PRESENT | flags | (ADDR_TO_PTE(pa));
        i++;
    }

    flush_tlb();
    return 0;
}



int vmcopy(vmem_t* dest, vmem_t* src) 
{
    uint32_t i, length, pa, va;
    pte_t* pte;
    char *cache, cachepa;
    vm_area_t* srcarea, *destarea, *nextarea;

    cachepa = get_user_page(0);
    cache = (char*) 0x7F00000;
    mmap((uint32_t)cache, cachepa, PAGE_SIZE, PTE_US | PTE_RW);

    dest->size = src->size;
    dest->start_brk = src->start_brk;
    dest->file_length = src->file_length;
    dest->brk = src->brk;
    //dest->count = ++src->count;

    srcarea = src->map_list;

    destarea = dest->map_list;
    while(destarea != 0) {
        nextarea = destarea->next;
        kfree(destarea);
        destarea = nextarea;
    }

    destarea = kmalloc(sizeof(vm_area_t));
    dest->map_list = destarea;

    while(srcarea != 0) {
        length = (srcarea->vmend - srcarea->vmstart) / PAGE_SIZE;

        destarea->mmap = kmalloc(sizeof(uint32_t*) * length);
        
        destarea->vmstart = srcarea->vmstart;
        destarea->vmend = srcarea->vmend;
        destarea->vmflag = srcarea->vmflag;

        i = 0;
        for(va = srcarea->vmstart; va < srcarea->vmend; va += PAGE_SIZE) {
            if((pte = _walk(va, 0, 0)) == 0) {
                panic("vmcopy: walk error");
            }
            if((*pte & PTE_PRESENT) == 0) {
                panic("vmcopy: src not present");
            }
            if((pa = get_user_page(0)) == 0) {
                panic("vmcopy: get user page failed");
            }
            
            memcpy(cache, (char*)va, PAGE_SIZE);

            freemap(va, PAGE_SIZE);

            if(mmap(va, pa, PAGE_SIZE, GETBIT_12(srcarea->mmap[i])) == -1) {
                free_user_page(pa, 0);
                panic("vmcopy: remap failed");
            }
            memcpy((char*)va, cache, PAGE_SIZE);

            destarea->mmap[i] = PTE_PRESENT | GETBIT_12(srcarea->mmap[i]) | (ADDR_TO_PTE(pa));
            i ++;
        }

        srcarea = srcarea->next;
        if(srcarea != 0) {
            nextarea = kmalloc(sizeof(vm_area_t));
            destarea->next = nextarea;
            destarea = nextarea;
        }
    }

    destarea->next = 0;

    freemap((uint32_t)cache, PAGE_SIZE);
    free_user_page(cachepa, 0);

    return 0;
}


/*
int vmdealloc(vmem_t* vm, int oldsize, int newsize)
{
    uint32_t startva, endva, va, pa;
    startva = ADDR_TO_PTE(newsize) + PAGE_SIZE;
    endva = ADDR_TO_PTE(oldsize);

    _freemap(pd, va, (startva - endva) / PAGE_SIZE);

    return;
}
*/

