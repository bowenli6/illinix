===================================
Programmable Interrupt Controller
===================================

--------------------
Description
--------------------
The Intel 8259A Programmable Interrupt Controller handles up to eight vectored 
priority interrupts for the CPU It is cascadable for up to 64 vectored priority 
interrupts without additional circuitry It is packaged in a 28-pin DIP uses NMOS 
technology and requires a single a 5V supply Circuitry is static requiring no clock input.

The Programmable Interrupt Controller (PIC) functions as an overall manager in 
an Interrupt-Driven system environment It accepts requests from the peripheral 
equipment determines which of the incoming requests is of the highest importance 
(priority) ascertains whether the incoming request has a higher priority value than 
the level currently being serviced and issues an interrupt to the CPU based on this determination.


--------------------
Source code
--------------------
student-distrib/include/boot/i8259.h

student-distrib/kernel/i8259.c
