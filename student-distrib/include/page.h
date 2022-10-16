#ifndef _PAGE_H
#define _PAGE_H

#define PDE_OFFSET_4KB 12
#define VIDEO       0xB8000

void page_init();
void enable_paging();

#endif /* _PAGE_H */
