#ifndef PAGE_H
#define PAGE_H

#define PDE_OFFSET_4KB 12
#define VIDEO       0xB8000

void page_init();
void enable_paging();

#endif /* PAGE_H */
