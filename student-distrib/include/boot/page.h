#ifndef _PAGE_H
#define _PAGE_H

#define PDE_OFFSET_4KB      12
#define VIDEO               0xB8000
#define CR4_EXTENSION_FLAG  0x10
#define CR0_PAGE_FLAG       0x80000000


void page_init();
void enable_paging();

#endif /* _PAGE_H */
