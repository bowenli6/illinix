#include <lib.h>
#include <kmalloc.h>
#include <boot/x86_desc.h>
#include <boot/page.h>
 


free_area_t free_area[MAX_ORDER + 1];
buddy free_list[MAX_ORDER + 1];

uint32_t bit_map[MAX_BMSIZE * 2 + 12];

page_header_t *page_header;
page_header_t ph;
slab_t l1;
slab_t *slab_free_list;
slab_t *start_slab;

#define PAGE_SLAB (PAGE_SIZE_4MB / SLAB_SIZE)


int add_header(void* pt, int order);
int pop_header(int addr);
void slab_list_push(slab_t* list, slab_t* s);
void slab_list_remove(slab_t* b);

slab_t* _slab_free(slab_t* ns); 
int slab_space_grow();
void bminit();
buddy* get_buddy(uint32_t addr);



/**
 * @brief Initialize required data structures,
 * and indexing the available memory in kernel space.
 * 
 */
void kmalloc_init()
{
    int i = 0;
    buddy* p;

    for(i = 0; i <= MAX_ORDER; i++) {
        free_list_init(i); 
        bminit();
        free_area[i].start_addr = RESERVED_PAGES * PAGE_SIZE_4MB;
        free_area[i].user = 0;
        //memset(free_area[i].bit_map, 0, 16 * sizeof(uint32_t)); /* All buddys are both free */
    }

    buddy* fl = free_area[MAX_ORDER].free_list;
    /* index kernel memory and store in largest free list */
    for(i = RESERVED_PAGES * PAGE_SIZE_4MB; i < KERNEL_PAGES * PAGE_SIZE_4MB; i += buddy_size(MAX_ORDER - 1)) {
        p = (buddy*) i;
        p->addr = i;
        free_list_push(fl, p);
    }
    /* initialize page alloc header */
    page_header = &ph;
    page_header->next = page_header;

    slab_init();

    return;
}

/**
 * @brief Initialize kernel memory bit map for buddy allocator.
 * 
 */
void bminit()
{
    int i, t = 0;
    memset(bit_map, 0, sizeof(uint32_t) * (MAX_BMSIZE * 2 + 10));
    for(i = 0; i < MAX_ORDER; i++) {
        free_area[i].bmsize = (MAX_BMSIZE >> i) + (i > 4 ? 1 : 0);
        free_area[i].bit_map = bit_map + t;
        t += free_area[i].bmsize;
    }
}

/**
 * @brief Allocate parameter size kernel memory space.
 * Use the slab allocator for allocations under 4KB, 
 * and use the buddy allocator directly for 4KB and above.
 * 
 * @param size Required memory space, can be any integer from 0 to 4MB.
 * @return void* pointer to allocated memory space. NULL means failed.
 */
void* kmalloc(int size) 
{   
    int order = 0, temp = 1;
    char* pt;

    if (size <= 0) return NULL;

    if(size > PAGE_SIZE_4MB) /* larger than upper limit */
        return NULL;

    /* use slab allocator for small size */
    if((size + sizeof(slab_t)) < PAGE_SIZE) {
        return slab_alloc(size);

    } else {
        /* calculate order for get_page */
        size = (size + PAGE_SIZE - 1) / PAGE_SIZE;
        while(temp < size) {
            temp *= 2;
            order ++;
        }
        if((pt = get_page(order)) == 0) 
            return 0;
        if(add_header(pt, order) == -1) /* add page alloc header */
            return 0;
        return (void*)pt;
    }
}

/**
 * @brief add kmalloc header for kmalloc request >= 4KB
 * 
 * @param pt allocated space
 * @param order order of page alloc
 * @return 0 means succeed, -1 means failed
 */
int add_header(void* pt, int order)
{
    page_header_t* h;
    /* use dynamic space to manage page alloc headers */
    if((h = kmalloc(sizeof(page_header_t))) == 0)
        return -1;
    h->addr = (uint32_t)pt;
    h->order = order;
    h->next = page_header->next;
    page_header->next = h;
    return 0;
}

/**
 * @brief free a kernel space created by kmalloc
 * 
 * @param p pointer to the space
 */
void kfree(void* p)
{
    int t;

    /* do nothing when p is NULL */
    if (!p) return;
    
    if((t = pop_header((uint32_t)p)) != -1) {   /* try to use free_page */
        free_page(p, t);
        return;
    }
    
    slab_t* header = (slab_t*)((uint32_t)p - sizeof(slab_t));
    if(header->status == 0) return; /* double free a pointer */
    if((header->size + sizeof(slab_t)) < PAGE_SIZE) {
        slab_free(header);
    }
    return;
}

/**
 * @brief Try to delete a kmalloc header.
 * 
 * @param addr space to free
 * @return order to free_page; -1 means using slab.
 */
int pop_header(int addr) 
{
    page_header_t* h = page_header, *nh;
    int t;
    while(h->next != page_header) {
        if(h->next->addr == addr) {
            nh = h->next;
            t = nh->order;
            h->next = nh->next;
            kfree(nh);
            return t;
        }
        h = h->next;
    }
    return -1;
}

/**
 * @brief Allocate a kernel space of size 2^(order) of 4KB.
 * 
 * @param order The minimum order is 0 (4KB) and the maximum order is 10 (4MB).
 * @return void* pointer to allocated space
 */
void* get_page(int order)
{
    int i;
    buddy* temp;
    if(order >= MAX_ORDER || order < 0) return NULL;

    for(i = order; i <= MAX_ORDER; i++) {
        /* find the smallest order that has a free block */ 
        if((temp = free_list_pop(free_area[i].free_list)) == NULL) {
            continue;
        }
        return (void*)buddy_split(free_area, temp, i, order); /* reduce order with split */
    }

    return NULL;
}

/**
 * @brief Recursively split the blocks until obtain a 
 *        memory block of proper size.
 * 
 * @param b Block to split.
 * @param cur_order order of buddy b.
 * @param tar_order required order of get_page().
 * @return buddy* memory generated by split.
 */
buddy* buddy_split(free_area_t* area, buddy* b, int cur_order, int tar_order)
{
    /* b2: pointer to the right child block of b on a lower order */
    buddy* b2;
    if(area->user){
        b2 = get_buddy(b->addr + buddy_size(cur_order) / 4);
    }
    else{
        b2 = (buddy*) (((uint32_t)b) + buddy_size(cur_order) / 4);
        b2->addr = (uint32_t)b2;
    }
    
    if(cur_order == tar_order) {
        /* Completely alloc a partially free buddy, or partially free a both free buddy */
        flip_bit_map(area, b, cur_order);
        return b;
    }
    if(cur_order != MAX_ORDER)
        flip_bit_map(area, b, cur_order);
    /* The right block of the next order will not be used for allocation */
    free_list_push(area[cur_order - 1].free_list, b2);
    
    return buddy_split(area, b, cur_order - 1, tar_order);
}

/**
 * @brief Determine if buddy b is partially free.
 * 
 * @param order order of b
 * @return 1 means has only one free block, 0 means both free or busy.
 */
int is_split(free_area_t* area, buddy* b, int order) 
{
    int index = (b->addr - area->start_addr) / buddy_size(order);
    if(area[order].bit_map[index / 32] & (1 << (index % 32)))
        return 1;
    else
        return 0;
}

/**
 * @brief Flip the partially busy pair bitmaps of buddy b. 
 * 
 * @param order order of b
 */
void flip_bit_map(free_area_t* area, buddy* b, int order) 
{
    int index = (b->addr - area->start_addr) / buddy_size(order);
    area[order].bit_map[index / 32] ^= (1 << (index % 32));
}

/**
 * @brief Get the size of a buddy.
 */
int buddy_size(int order) 
{
    return ((PAGE_SIZE * 2) << order);
}


/**
 * @brief Free a space created by get_page().
 * 
 * @param p pointer to the space
 * @param order The minimum order is 0 (4KB) and the maximum order is 10 (4MB).
 */
void free_page(void* p, int order)
{
    buddy* b = (buddy*) p;
    b->addr = (uint32_t) b;
    _free_page(free_area, b, order);
    return;    
}

/**
 * @brief Recursively free the allocated space and its parent buddies.
 * 
 * @param b current buddy
 * @param order current order
 */
void _free_page(free_area_t* area, buddy* b, int order)
{
    buddy* t;
    if(order == MAX_ORDER){
        free_list_push(area[order].free_list, b);
        return;
    } 
    if(!is_split(area, b, order)) {
        free_list_push(area[order].free_list, b);
        flip_bit_map(area, b, order);
    }
    else {
        flip_bit_map(area, b, order);
        t = remove_from_free(area, area[order].free_list, b, order);
        _free_page(area, t, order + 1);
    }
    return;
}

/**
 * @brief Remove memory block b from a free list and merge it with its buddy block.
 * 
 * @param fl the free list b currently exist in.
 * @param order order of b
 */
buddy* remove_from_free(free_area_t* area, buddy* fl, buddy* b, int order)
{
    uint32_t b2, left;
    // if(is_left_buddy(area, b, order))
    //     b2 = (((uint32_t)b) + buddy_size(order) / 2);
    // else
    //     b2 = (((uint32_t)b) - buddy_size(order) / 2);

    left = is_left_buddy(area, b, order);
    if(left)
        b2 = b->addr + buddy_size(order) / 2;
    else
        b2 = b->addr - buddy_size(order) / 2;

    buddy *t = fl->next;
    while(t != fl) {
        /* try to merge buddy blocks */
        if(t->addr == b2) {
            t->last->next = t->next;
            t->next->last = t->last;
            if(left){
                if(area->user) 
                    kfree(t);
                return b;
            }
            else{
                if(area->user) 
                    kfree(b);
                return t;
            }
        }
        t = t->next;
    }
    return NULL;
}


/**
 * @brief Determine whether d is pointing to the memory block on the left 
 * or the memory block on the right in a buddy under the current order.
 * 
 * @return 1 means left memory block, 0 means right memory block.
 */
int is_left_buddy(free_area_t* area, buddy* b, int order) 
{
    int t;
    if(area->user)
        t = ((b->addr - area->start_addr) / (buddy_size(order) / 2));
    else
        t = (((uint32_t)b - area->start_addr) / (buddy_size(order) / 2));
    return (t % 2 == 0) ? 1 : 0; 
}

/**
 * @brief Initialize required data structures of slab allocator.
 * 
 */
void slab_init()
{
    slab_t *s;
    slab_free_list = &l1;
    slab_free_list->next = slab_free_list;
    slab_free_list->last = slab_free_list;
    slab_free_list->size = 0;

    s = get_page(MAX_ORDER - 1);
    s->size = PAGE_SLAB;
    s->status = 0;
    start_slab = s;
    slab_list_push(slab_free_list, s);
    return;
}

/**
 * @brief Allocate a small kernel space using slab allocator.
 * 
 * @param size must be an integer between 0 and (4KB - sizeof(slab_t)).
 * @return void* pointer to allocated space
 */
void* slab_alloc(int size)
{
    size += sizeof(slab_t);
    slab_t* s = slab_free_list->next, *ns;
    int size_n = (size + SLAB_SIZE - 1) / SLAB_SIZE; /* required number of slabs */

    while(s != slab_free_list) {
        /* looking for a slab with enough space */
        if(s->size >= size_n) {
            slab_list_remove(s);
            break;
        }
        s = s->next;
    }

    if(s == slab_free_list) {
        /* All slabs do not have enough space, allocate a new page */
        if(slab_space_grow() == 0)
            return 0;
        else
            return slab_alloc(size - sizeof(slab_t));
    }

    s->status = 1;
    if(s->size == size_n) {
        return (void*)(((uint32_t)s) + sizeof(slab_t));
    } else {
        /* put the rest of the space back */
        ns = (slab_t*) (((uint32_t)s) + size_n * SLAB_SIZE);
        ns->size = s->size - size_n;
        ns->status = 0;
        s->size = size_n;
        slab_list_push(slab_free_list, ns);

        return (void*)(((uint32_t)s) + sizeof(slab_t));
    }
}

/**
 * @brief Add a new 4kb page to the slab allocator. 
 * Called when contiguous space is not enough.
 * 
 * @return 0 means succeed, -1 means failed.
 */
int slab_space_grow() 
{   
    slab_t* s;
    if((s = get_page(MAX_ORDER - 1)) == 0) 
        return -1;
    s->size = PAGE_SLAB;
    s->status = 0;
    slab_list_push(slab_free_list, s);
    return 0;
}


/**
 * @brief Free a space created by slab_alloc().
 * 
 * @param p pointer to the space
 */
void slab_free(slab_t* p)
{
    slab_t *ns = (slab_t*) p;
    //slab_list_remove(ns);
    ns = _slab_free(ns);
    ns->status = 0;

    /* If there is a extra empty page, free it */
    if((ns != start_slab) && !((uint32_t)ns % PAGE_SIZE_4MB) && (ns->size == PAGE_SLAB)) {
        free_page((void*)ns, (MAX_ORDER - 1));
    }
    slab_list_push(slab_free_list, ns);
    return;
}

/**
 * @brief Recursively merge adjacent frees space in a page.
 * 
 * @param ns pointer to current memory block
 * @return slab_t* Finally merged free space
 */
slab_t* _slab_free(slab_t* ns)
{
    slab_t *s = slab_free_list->next;
    while(s != slab_free_list) {
        /* try to merge adjacent space */
        if(((uint32_t)s + s->size * SLAB_SIZE == (uint32_t)ns 
        || (uint32_t)ns + ns->size * SLAB_SIZE == (uint32_t)s)
        && (((uint32_t)s % (PAGE_SIZE_4MB)) 
        || ((uint32_t)ns % (PAGE_SIZE_4MB)))) {
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


/* The following functions are helper functions for the linked lists.*/

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


