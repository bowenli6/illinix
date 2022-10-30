#ifndef _KMALLOC_H
#define _KMALLOC_H

#define PAGE_SIZE 0x1000
#define RESERVED_PAGES 3
#define USER_START_ADDR 0x80000000
#define MAX_PAGES 16


void kmalloc_init(void);
void* kmalloc(void);
void kfree(void* p);

void page_alloc_init(void);
void* page_alloc(void);
void page_free(void* pt);

struct mem_block
{
    struct mem_block* next;
};
typedef struct mem_block mem_block;

typedef struct dmem_t {
    mem_block* free_list;
    mem_block* alloc_list;
} dmem_t;

#endif /* _KMALLOC_H */
