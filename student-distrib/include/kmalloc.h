#ifndef _KMALLOC_H
#define _KMALLOC_H

#define RESERVED_PAGES 2
#define USER_START_ADDR 0x80000000
#define MAX_ORDER 11

#define SLAB_SIZE 0x08

#define MAX_BMSIZE 224
#define BIT_MAP_COMP(x) (1L << (x % 32))


struct mem_block
{
    struct mem_block* next;
};
typedef struct mem_block mem_block;

typedef struct dmem_t {
    mem_block* free_list;
    mem_block* alloc_list;
} dmem_t;


struct buddy
{
    struct buddy* next;
    struct buddy* last;
    uint32_t addr; /* used in user memory alloc */
};
typedef struct buddy buddy;

typedef struct free_area_t {
    buddy* free_list;
    uint32_t* bit_map;
    int bmsize;
    uint32_t start_addr;
    int user;
} free_area_t;

typedef struct page_header_t {
    int addr;
    int order;
    struct page_header_t* next;
} page_header_t;

typedef struct slab_t {
    int size;
    int status;
    struct slab_t* last;
    struct slab_t* next;
} slab_t;

void kmalloc_init(void);
void* kmalloc(int size);
void kfree(void* p);
void* get_page(int order);
void free_page(void* p, int order);
void _free_page(free_area_t* area, buddy* b, int order);

void slab_init(void);
void* slab_alloc(int size);
void slab_free(slab_t* p);

void free_list_init(int order);
void free_list_push(buddy* fl, buddy* b);
buddy* free_list_pop(buddy* fl);
buddy* buddy_split(free_area_t* area, buddy* b, int cur_order, int tar_order);
int buddy_size(int order);
int is_split(free_area_t* area, buddy* b, int order);
void flip_bit_map(free_area_t* area, buddy* b, int order);
int is_left_buddy(free_area_t* area, buddy* b, int order);
buddy* remove_from_free(free_area_t* area, buddy* fl, buddy* b, int order);


#endif /* _KMALLOC_H */
