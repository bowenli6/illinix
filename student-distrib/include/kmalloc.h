#ifndef _KMALLOC_H
#define _KMALLOC_H

#define RESERVED_PAGES 2
#define USER_START_ADDR 0x80000000
#define MAX_PAGES 14
#define SLAB_PAGES 2
#define MAX_ORDER 11

#define SLAB_SIZE 0x200

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

typedef struct alloc_header_t {
    int addr;
    int size;
} alloc_header_t;

typedef struct slab_t {
    int size;
    int addr;
    int status;
    struct slab_t* last;
    struct slab_t* next;
    //alloc_header_t* info;
} slab_t;

void kmalloc_init(void);
void* kmalloc(int size);
void kfree(void* p);
void* get_page(int order);
void free_page(void* p, int order);

void page_alloc_init(void);
void* page_alloc(void);
void page_free(void* p);
int buddy_size(int order);

void slab_init(void);
void* slab_alloc(int size);
void slab_free(slab_t* p);


#endif /* _KMALLOC_H */
