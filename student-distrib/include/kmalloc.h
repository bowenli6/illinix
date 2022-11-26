#ifndef _KMALLOC_H
#define _KMALLOC_H

#define RESERVED_PAGES 2
#define USER_START_ADDR 0x80000000
#define MAX_PAGES 12
#define SLAB_PAGES 4
#define MAX_ORDER 11

#define SLAB_SIZE 0x08

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
};
typedef struct buddy buddy;

typedef struct free_area_t {
    buddy* free_list;
    uint32_t bit_map[128];
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
int buddy_size(int order);

void slab_init(void);
void* slab_alloc(int size);
void slab_free(slab_t* p);


#endif /* _KMALLOC_H */
