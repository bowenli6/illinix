#ifndef _PAGE_H
#define _PAGE_H

#define PDE_OFFSET_4KB      12
#define VIDEO               0xB8000
#define CR4_EXTENSION_FLAG  0x10
#define CR0_PAGE_FLAG       0x80000000


void page_init();
void enable_paging();
pte_t* walk(uint32_t va, int alloc);
int _mmap(uint32_t va, uint32_t pa, int size, int rw, int us);
void kmmap(uint32_t va, uint32_t pa, int size, int rw, int us);
void ummap(uint32_t va, uint32_t pa, int size, int rw, int us);


typedef uint32_t* pagetable_t;

#endif /* _PAGE_H */
