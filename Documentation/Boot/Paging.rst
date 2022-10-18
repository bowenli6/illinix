=======================================
Paging
=======================================

------------------
Description 
------------------
Paging is a memory management scheme that mapping physical memory to
virtual memory pages. The paging of OS has two types: 4M-Bytes and 
4K-Bytes.
4M-Bytes page directory has two levels, each page directory entry maps 
to a 4M-Byte physical memory block.
4K-Bytes page directory has three levels, each page directory entry maps
to a page table, and each page table maps to a 4K-Bytes physical memory block.

The first 4MB virtual memory is set to 4K-Bytes type, and the following 
virtual memory including 4MB-8MB is set to 4M-Bytes type. The first block of memory
contains the video memory.

-------------------
Page Structure
-------------------
1. Page_directory[1024]{KB, MB}

2. Page_table[1024]


---------------------
Page Initialize
---------------------
Set the page_directory[0].present and page_directory[1].present to 1, and others to 0.

Set the base address of page_directory[0] to page table address.

Set the page_size and global flag of page_directory[1].

Set the page table entry of video memory to present and others to not present.


---------------------
Enable Paging
---------------------
CR3: page directory base register
CR4 bit 4: page extension flag
CR0 bit 31: paging flag

Store page directory base address into CR3, 
set CR4 bit 4 to 1, then set CR0 bit 31 to 1.

--------------------
Source Code
--------------------
student-distrib/include/boot/page.h

student-distrib/kernel/page.c

student-distrib/include/boot/x86_desc.h

student-distrib/x86_desc.S







