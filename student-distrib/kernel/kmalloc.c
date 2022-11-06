#include <lib.h>
#include <kmalloc.h>
#include <boot/page.h>

dmem_t dmem;

/**
 * @brief initialize dynamic memory
 *
 */
void page_alloc_init()
{
    int i = 0;
    mem_block* p;
    dmem.free_list = NULL;
    dmem.alloc_list = NULL;
    for(i = RESERVED_PAGES * PAGE_SIZE ; i < MAX_PAGES * PAGE_SIZE; i++){
        p = (mem_block*) (i * PAGE_SIZE);
        memset((char*)p, 0, PAGE_SIZE);
        p->next = dmem.free_list;
        dmem.free_list = p;
    }
    return;
}

/**
 * @brief allocate a 4KB memory
 * @return pointer to the allocated memory
 * 
 */
void* page_alloc()
{
    mem_block* p;
    p = dmem.free_list;
    if(p == NULL) return NULL;
    dmem.free_list = p->next; /* remove from free list */
    p->next = dmem.alloc_list; /* add to alloc list */
    dmem.alloc_list = p;
    return p;
}

/**
 * @brief free a 4KB memory
 * 
 */
void page_free(void* pt)
{
    mem_block* p = (mem_block*) pt, *temp;

    /* legal free judgement */
    temp = dmem.alloc_list;
    if(temp == NULL) //panic("Free error occur!");
        return;
    if(temp != p){
        while(temp->next != p){
            if(temp->next == NULL) //panic("Free error occur!");
                return;
            temp = temp->next;
        }
        temp->next = temp->next->next;  /* remove from alloc list */
    }
    else dmem.alloc_list = temp->next;
    

    memset((char*)p, 0, PAGE_SIZE);
    p->next = dmem.free_list;   /* add into free list */
    dmem.free_list = p;
    return;
}
