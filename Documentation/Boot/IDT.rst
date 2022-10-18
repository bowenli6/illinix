=======================================
Interrupt Descriptor Table
=======================================

------------------
Description 
------------------
The interrupt descriptor table (IDT) associates each exception or interrupt 
vector with a gate descriptor for the procedure or task used to service the 
associated exception or interrupt. Like the GDT and LDTs, the IDT is an array 
of 8-byte descriptors (in protected mode). Unlike the GDT, the first entry of 
the IDT may contain a descriptor. To form an index into the IDT, the processor 
scales the exception or interrupt vector by eight (the number of bytes in a gate descriptor). 
Because there are only 256 interrupt or exception vectors, the IDT need not contain more than 
256 descriptors. It can contain fewer than 256 descriptors, because descriptors are required 
only for the interrupt and exception vectors that may occur. All empty descriptor slots in the 
IDT should have the present flag for the descriptor set to 0.


-------------------
Gate Descriptor
-------------------
The IDT may contain any of three kinds of gate descriptors:

• **Task-gate descriptor**

• **Interrupt-gate descriptor**

• **Trap-gate descriptor**


---------------------
Task Gate
---------------------
An Intel task gate that cannot be accessed by a User Mode process 
(the gate’s DPL field is equal to 0). The handler for the “Double fault” 
exception is activated by means of a task gate.

---------------------
Interrupt Gate
---------------------
An Intel interrupt gate that cannot be accessed by a User Mode process 
(the gate’s DPL field is equal to 0). All interrupt handlers are activated 
by means of interrupt gates, and all are restricted to Kernel Mode.


---------------------
Trap Gate
---------------------
An Intel trap gate that cannot be accessed by a User Mode process 
(the gate’s DPL field is equal to 0). 
Most exception handlers are activated by means of trap gates.

--------------------
Handler
--------------------
An interrupt gate or trap gate references an exception or interrupt handler procedure 
that runs in the context of the currently executing task. The segment selector for the gate points
to a segment descriptor for an executable code segment in either the GDT or the current LDT. 
The offset field of the gate descriptor points to the beginning of the exception or interrupt handling procedure.

--------------------
Source Code
--------------------
student-distrib/include/boot/x86_desc.h

student-distrib/x86_desc.S

student-distrib/include/boot/exception.h

student-distrib/include/boot/interrupt.h

student-distrib/include/boot/syscall.h

student-distrib/kernel/handler.S

student-distrib/kernel/exception.c

student-distrib/kernel/syscall.c

