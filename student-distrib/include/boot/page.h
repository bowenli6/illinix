#ifndef _PAGE_H
#define _PAGE_H

#define PDE_OFFSET_4KB      12
#define PDE_OFFSET_4MB      22
#define VIDEO               0xB8000
#define CR4_EXTENSION_FLAG  0x10
#define CR0_PAGE_FLAG       0x80000000
#define KERNEL_INDEX 1
#define VIR_VID_MEM 0x8400000

#define PTE_PRESENT 0x1
#define PTE_RW 0x2
#define PTE_US 0x4
#define PTE_WT 0x8
#define PTE_CD 0x10
#define PTE_AC 0x20
#define PTE_DT 0x40
#define PTE_PAT 0x80
#define PDE_MB 0x80
#define PTE_GLO 0x100

#define PTE_ADDR(x) ((x) >> 12)
#define PDE_MB_ADDR(x) ((x) >> 22)

#define ADDR_TO_PTE(a) ((a) & 0xFFFFF000)
#define ADDR_TO_4MB(a) ((a) & 0xFFC00000)

#define INDEX_TO_DIR(x) ((x) << 22)


typedef uint32_t* pagetable_t;
typedef uint32_t* pagedir_t;


void page_init();
void enable_paging();
void flush_tlb();

int _mmap(pagedir_t pd, uint32_t va, uint32_t pa, int size, int flags);

void kmmap(uint32_t va, uint32_t pa, int size, int flags);

pagedir_t create_uvmdir();
int _get_pde_index();

int _freemap(pagedir_t pd, uint32_t va, int size);
void free_uvmdir(pagedir_t pd, int size);

int vmalloc(pagedir_t pd, int oldsize, int newsize, int flags);
void vmdealloc(pagedir_t pd, int oldsize, int newsize);

int32_t do_vidmap(uint8_t **screen_start);

typedef struct pg_descriptor_t {
    uint32_t flags;
    int _count;
}pd_descriptor_t;


#endif /* _PAGE_H */
