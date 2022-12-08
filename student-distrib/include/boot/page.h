#ifndef _PAGE_H
#define _PAGE_H

#include<pro/process.h>

#define PDE_OFFSET_4KB      12
#define PDE_OFFSET_4MB      22
#define VA_OFFSET           12
#define GETBIT_10           0x3FF
#define VIDEO               0xB8000

#define VIDEO_BUF_1         0xD0000
#define VIDEO_BUF_2         0xD1000
#define VIDEO_BUF_3         0xD2000

#define CR4_EXTENSION_FLAG  0x10
#define CR0_PAGE_FLAG       0x80000000
#define KERNEL_INDEX        1
#define USER_MEM            0x8000000
#define VIR_VID_MEM         0x8400000
#define HEAP_START          0x8800000
#define KERNEL_PAGES        16
#define MAX_PHYS_PAGES      64


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

#define VM_EXEC 0x01
#define VM_WRITE 0x02
#define VM_READ 0x04
#define VM_HEAP 0x08
#define VM_STACK 0x010

#define PTE_ADDR(x) ((x) >> 12)
#define PDE_MB_ADDR(x) ((x) >> 22)

#define ADDR_TO_PTE(a) ((a) & 0xFFFFF000)
#define ADDR_TO_4MB(a) ((a) & 0xFFC00000)

#define GETBIT_12(a) ((a) & 0xFFF )

#define INDEX_TO_DIR(x) ((x) << 22)

typedef uint32_t* pagetable_t;
typedef uint32_t* pagedir_t;

typedef struct user_page_t {
    uint32_t addr;
    struct user_page_t* next;
    struct user_page_t* last;
} user_page_t;

void page_init();
void enable_paging();
void flush_tlb();

void do_mmap(int size);

int mmap(uint32_t va, uint32_t pa, int size, int flags);
int freemap(uint32_t va, int size);
void free_uvmdir(int size);

void process_vm_init(vmem_t* vm);
int vmalloc(vm_area_t* vm, int incrsize, int flags);
void vmdealloc(vm_area_t* vm, int decsize, int mapping);
int vmcopy(vmem_t* dest, vmem_t* src);

int32_t do_vidmap(uint8_t **screen_start);

void user_mem_init();
uint32_t get_user_page(int order);
void free_user_page(uint32_t addr, int order);
void show_mmap(vmem_t* vm);

typedef struct pg_descriptor_t {
    uint32_t flags;
    int count;
}pd_descriptor_t;



#endif /* _PAGE_H */
