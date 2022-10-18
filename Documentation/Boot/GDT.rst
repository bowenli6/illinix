=============================================
Global and Local Descriptor Tables
=============================================

-------------------
Description
-------------------
When operating in protected mode, all memory accesses pass through either the **global descriptor table** (GDT) 
or the (optional) **local descriptor table** (LDT). These tables contain entries called 
segment descriptors. A segment descriptor provides the base address of a segment and access rights, type, 
and usage information. Each segment descriptor has a segment selector associated with it. The segment selector 
provides an index into the GDT or LDT (to its associated segment descriptor), a global/local flag (that determines 
whether the segment selector points to the GDT or the LDT), and access rights information.

To access a byte in a segment, a segment selector and an offset are supplied. The segment selector provides access 
to the segment descriptor for the segment (in the GDT or LDT). From the segment descriptor, the processor obtains 
the base address of the segment in the linear address space. The offset then provides the location of the byte 
relative to the base address. This mechanism can be used to access any valid code, data, or stack segment in the 
GDT or LDT, provided the segment is accessible from the current privilege level (CPL) at which the processor is operating. 
The CPL is defined as the protection level of the currently executing code segment.


-----------------
Source Code
-----------------
student-distrib/include/boot/x86_desc.h

student-distrib/x86_desc.S
