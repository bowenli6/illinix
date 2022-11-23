#include <lib.h>
#include <kmalloc.h>
#include <boot/x86_desc.h>
#include <boot/page.h>
 
dmem_t dmem;
free_area_t free_area[MAX_ORDER + 1];
buddy free_list[MAX_ORDER + 1];
slab_t l1, l2;
slab_t *slab_free_list;
slab_t *slab_alloc_list;

/**
 * @brief initialize dynamic memory
 *
 */

void free_list_init(int order) 
{
    buddy* fl = &free_list[order];
    free_area[order].free_list = fl;
    fl->last = fl;
    fl->next = fl;
}

void free_list_push(buddy* fl, buddy* b) 
{
    buddy* prev = fl->last;
    b->last = prev;
    b->next = fl;
    prev->next = b;
    fl->last = b;
}

buddy* free_list_pop(buddy* fl) 
{
    buddy* head = fl->next;
    if(head == fl) 
        return NULL;
    head->next->last = fl;
    fl->next = head->next;
    return head;
}


void kmalloc_init()
{
    int i = 0;
    buddy* p;

    for(i = 0; i <= MAX_ORDER; i++) {
        free_list_init(i);
        memset(free_area[i].bit_map, 0, 16 * sizeof(uint32_t));
    }

    buddy* fl = free_area[MAX_ORDER].free_list;
    for(i = RESERVED_PAGES * PAGE_SIZE_4MB; i < MAX_PAGES * PAGE_SIZE_4MB; i += buddy_size(MAX_ORDER - 1)) {
        p = (buddy*) i;
        free_list_push(fl, p);
    }
    slab_init();

    return;
}



/**
 * @brief 
 * 
 * @param size demanding memory size, no need to be page size
 * @return void* pointer to the allocated memory space
 */
void* kmalloc(int size) 
{
    int order = 0, temp = 1;
    if(size > PAGE_SIZE_4MB)
        return NULL;

    if((size + sizeof(slab_t)) < PAGE_SIZE) {
        return slab_alloc(size);
    } else {
        size = (size + PAGE_SIZE - 1) / PAGE_SIZE;
        while(temp < size) {
            temp *= 2;
            order ++;
        }
        return get_page(order);
    }
}

int buddy_size(int order) {
    //int size = PAGE_SIZE * 2;
    return ((PAGE_SIZE * 2) << order);
}

int is_split(buddy* b, int order) {
    int index = ((uint32_t)b) / buddy_size(order);
    if(free_area[order].bit_map[index / 32] & BIT_MAP_COMP(index))
        return 1;
    else
        return 0;
}

void flip_bit_map(buddy* b, int order) {
    int index = ((uint32_t)b) / buddy_size(order);
    free_area[order].bit_map[index / 32] ^= BIT_MAP_COMP(index);
}

buddy* buddy_split(buddy* b, int cur_order, int tar_order)
{
    buddy* b2 = (buddy*) (((uint32_t)b) + buddy_size(cur_order) / 4);
    if(cur_order == tar_order) {
        //free_list_push(free_area[cur_order].free_list, b2);
        flip_bit_map(b, cur_order);
        return b;
    }
    free_list_push(free_area[cur_order - 1].free_list, b2);
    if(cur_order != MAX_ORDER) flip_bit_map(b, cur_order);
    return buddy_split(b, cur_order - 1, tar_order);
}

void* get_page(int order)
{
    int i;
    buddy* temp;
    if(order >= MAX_ORDER) return NULL;
    // if(order == MAX_ORDER) {
    //     temp = free_list_pop(free_area[MAX_ORDER].free_list);
    //     return (void*)temp;
    // }
    for(i = order; i <= MAX_ORDER; i++) {
        if((temp = free_list_pop(free_area[i].free_list)) == NULL) {
            continue;
        }
        return buddy_split(temp, i, order);
    }
    return NULL;
}



void slab_list_push(slab_t* list, slab_t* s)
{
    slab_t* prev = list->last;
    s->last = prev;
    s->next = list;
    prev->next = s;
    list->last = s;
}

void slab_list_remove(slab_t* b)
{
    b->last->next = b->next;
    b->next->last = b->last;
}

void slab_init()
{
    slab_t *s;
    slab_free_list = &l1;
    slab_alloc_list = &l2;
    slab_free_list->next = slab_free_list;
    slab_free_list->last = slab_free_list;
    slab_alloc_list->next = slab_alloc_list;
    slab_alloc_list->last = slab_alloc_list;
    slab_free_list->size = 0;
    slab_alloc_list->size = 0;
    s = (slab_t*) (MAX_PAGES * PAGE_SIZE_4MB);
    s->size = SLAB_PAGES * PAGE_SIZE_4MB / SLAB_SIZE;
    s->status = 0;
    slab_list_push(slab_free_list, s);
    
    return;
}

void* slab_alloc(int size)
{
    size += sizeof(slab_t);
    slab_t* s = slab_free_list->next, *ns;
    int size_n = (size + SLAB_SIZE - 1) / SLAB_SIZE;
    while(s != slab_free_list) {
        if(s->size >= size_n) {
            slab_list_remove(s);
            break;
        }
        s = s->next;
    }
    if(s == slab_free_list) 
        return 0;
    
    if(s->size == size_n) {
        s->status = 1;
        slab_list_push(slab_alloc_list, s);
        return (void*)(((uint32_t)s) + sizeof(slab_t));
    }
    else {
        ns = (slab_t*) (((uint32_t)s) + size_n * SLAB_SIZE);
        ns->size = s->size - size_n;
        ns->status = 0;
        slab_list_push(slab_free_list, ns);
        s->size = size_n;
        s->status = 1;
        slab_list_push(slab_alloc_list, s);
        return (void*)(((uint32_t)s) + sizeof(slab_t));
    }
}

void kfree(void* p)
{
    slab_t* header = (slab_t*)((uint32_t)p - sizeof(slab_t));
    if(header->status == 0) return;
    if((header->size + sizeof(slab_t)) < PAGE_SIZE) {
        slab_free(header);
    }
    return;
}

int is_left_buddy(buddy* b, int order) 
{
    int t = (((uint32_t)b - RESERVED_PAGES * PAGE_SIZE_4MB) / (buddy_size(order) / 2));
    return (t % 2 == 0) ? 1 : 0; 
}

void remove_from_free(buddy* fl, buddy* b, int order)
{
    buddy* b2;
    if(is_left_buddy(b, order))
        b2 = (buddy*) (((uint32_t)b) + buddy_size(order) / 2);
    else
        b2 = (buddy*) (((uint32_t)b) - buddy_size(order) / 2);

    buddy *t = fl->next;
    while(t != fl) {
        if(t == b2) {
            t->last->next = t->next;
            t->next->last = t->last;
            return;
        }
        t = t->next;
    }
}

void _free_page(buddy* b, int order)
{
    if(order == MAX_ORDER){
        free_list_push(free_area[order].free_list, b);
        return;
    } 
    if(!is_split(b, order)) {
        free_list_push(free_area[order].free_list, b);
        flip_bit_map(b, order);
    }
    else {
        remove_from_free(free_area[order].free_list, b, order);
        flip_bit_map(b, order);
        _free_page(b, order + 1);
    }
    return;
}

void free_page(void* p, int order)
{
    buddy* b = (buddy*) p;
    _free_page(b, order);
    return;    
}

slab_t* _slab_free(slab_t* ns)
{
    slab_t *s = slab_free_list->next;
    while(s != slab_free_list) {
        if((uint32_t)s + s->size * SLAB_SIZE == (uint32_t)ns || (uint32_t)ns + ns->size * SLAB_SIZE == (uint32_t)s) {
            slab_list_remove(s);
            if((uint32_t)s > (uint32_t)ns) {
                ns->size = s->size + ns->size;
                return _slab_free(ns);
            } else {
                s->size = s->size + ns->size;
                return _slab_free(s);
            }
        }
        s = s->next;
    }
    return ns;
}

void slab_free(slab_t* p)
{
    slab_t *ns = (slab_t*) p;
    slab_list_remove(ns);
    ns = _slab_free(ns);
    ns->status = 0;
    slab_list_push(slab_free_list, ns);
    return;
}



/*
void page_alloc_init()
{
    int i = 0;
    mem_block* p;
    dmem.free_list = NULL;
    dmem.alloc_list = NULL;
    for(i = RESERVED_PAGES * PAGE_SIZE ; i < MAX_PAGES * PAGE_SIZE; i++){
        p = (mem_block*) (i * PAGE_SIZE);
        p->next = dmem.free_list;
        dmem.free_list = p;
    }
    return;
}


void* page_alloc()
{
    mem_block* p;
    p = dmem.free_list;
    if(p == NULL) return NULL;
    dmem.free_list = p->next; 
    p->next = dmem.alloc_list;
    dmem.alloc_list = p;
    return p;
}


void page_free(void* pt)
{
    mem_block* p = (mem_block*) pt, *temp;

    temp = dmem.alloc_list;
    if(temp == NULL)
        return;
    if(temp != p){
        while(temp->next != p){
            if(temp->next == NULL)
                return;
            temp = temp->next;
        }
        temp->next = temp->next->next;
    }
    else dmem.alloc_list = temp->next;
    

    p->next = dmem.free_list;
    dmem.free_list = p;
    return;
}
*/
