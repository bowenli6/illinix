# Illinix
## 1 Introduction
This project was initially designed as a team project for the ECE 391 Computer Systems Engineering course, with the aim of developing the core of a UNIX-like operating system. The basic requirements for the project included the core subsystems of the OS kernel, such as process management, virtual memory abstraction, scheduling, file system, and multiple device drivers. 

Additionally, two of the kernel developers on the team went beyond the scope of the course project and expanded the kernel's features by adding a sophisticated multitasking scheduler, memory management subsystem, and more than ten system calls. As a result of the team's design and implementation efforts, we won the Operating System Design competition sponsored by Microsoft. 

The goal of this project was to provide us with hands-on experience in developing systems software used to interface between devices and applications, i.e., operating systems. Writing an operating system requires a deep understanding of computer systems, including the hardware and software components that make up a computer. This knowledge can be valuable for future projects and can help us become better software engineers. Group projects require collaboration and communication between team members. Working in a team can help us develop skills in collaboration, communication, and conflict resolution that are essential for success in many professional settings. After completing this project, we gained the particular skills necessary to pursue our careers in computer systems engineering.

## 2 Environment
This section describes how to build your new OS which will be embedded, together with the ``filesys.img``, in ``mp3.img``. 

### 2.1 Running with QEMU
```qemu-system-i386 -hda "student-distrib/mp3.img" -m 256 -name test -gdb tcp:127.0.0.1:1234```
### 2.2 Debugging with QEMU
Whenever a change is made to the kernel or file system, you need to ``sudo make`` a new kernel, which consequently prepares the ``mp3.img`` file which is used in the newly modified test debug.lnk file.

Your new test ``debug.lnk`` file should be modified to pass the new mp3 QEMU image. Change the target line to ``"c:\qemu-1.5.0-win32-sdl\qemu-system-i386w.exe" -hda "<mp3 directory>\mp3.img" -m 256 -gdb tcp:127.0.0.1:1234 -S -name mp3``

You may also write the following to a ``.bat`` file to accomplish the same thing:
```
c:
cd "c:\qemu-1.5.0-win32-sdl\"
qemu-system-i386w.exe -hda <mp3 directory>\mp3.img -m 256 -gdb tcp:127.0.0.1:1234 -S -name mp3
```
where ``<mp3 directory>`` is likely to be: ``z:\mp3\student-distrib``. In order to start debugging with GDB, run your test debug.lnk file, then issue the following commands in the development machine:
```
cd <mp3 directory>/student-distrib gdb bootimg
target remote 10.0.2.2:1234
```
GDB should now be started and connected to QEMU. From here, you can set up break points and everything else that you would normally do with GDB. Type c to continue execution instead of r since you connected to QEMU which is already running. When you do continue execution in GDB, GRUB will load first in QEMU. You need to hit enter, or wait 5 seconds, for your OS to load. QEMU is known to crash if your page tables are incorrectly setup. You might have to use the task manager in Windows to kill QEMU if this happens.

### 2.3 ``/mnt/tmpmp3`` Compile Error
When compiling your kernel in MP3, you must first close the test machine. If you forget to close the test machine while compiling, or forget to run as root when you make, you will need to remove the old MP3 image before you can compile again. The commands below can be added to a script to remove the broken files and correct the issue (the last command should be run in git-bash).
```
sudo rm -rf /mnt/tmpmp3
rm bootimg
rm mp3.img
git checkout mp3.img
```
If you run the above steps and you still can’t compile, the mp3.img in your repo is broken. You will need to re- vert back to an earlier version of mp3.img or you will need to grab a fresh mp3.img from the class directory under ``mp3/student-distrib``.

### 2.4 Buffer I/O error on device ``loop0``, logical block #### lost page write due to I/O error on ``loop0``
While this error may be caused by many issues, it’s likely it was caused by an ``mp3.img`` corruption. Follow the above steps to attempt a fix.

## 3 CPU Initialization
The kernel currently only support Intel x86-32 processors.
### 3.1 Global and Local Descriptor Tables
When operating in protected mode, all memory accesses pass through either the global descriptor table (GDT) or the (optional) local descriptor table (LDT). These tables contain entries called segment descriptors. A segment descriptor provides the base address of a segment and access rights, type, and usage information. Each segment descriptor has a segment selector associated with it. The segment selector provides an index into the GDT or LDT (to its associated segment descriptor), a global/local flag (that determines whether the segment selector points to the GDT or the LDT), and access rights information.

Header files: 
```
student-distrib/include/multiboot.h
student-distrib/include/x86_desc.h
```
Source files:
```
student-distrib/kernel/x86_desc.S
student-distrib/boot.S
```

### 3.2 Interrupt Descriptor Tables
IDT contains entries for exceptions, a few interrupts, and system calls. Consult the x86 ISA manuals for the descriptor formats and please see ``System Calls`` section for more information. The exception handlers, except for the ``Page Fault Exception``, use the printing support to report errors when an exception occurs in the kernel, and squash any user-level program that produces an exception, returning control to the shell (the shell should not cause an exception in a working OS)— see ``System Calls`` section for further details. Interrupts for the keyboard and the RTC.

Header files: 
```
student-distrib/include/idt.h
student-distrib/include/interrupt.h
student-distrib/include/exception.h
student-distrib/include/syscall.h
student-distrib/include/x86_desc.h
```
Source files:
```
student-distrib/kernel/handler.S
student-distrib/kernel/exception.c
student-distrib/kernel/syscall.c
student-distrib/kernel/x86_desc.S
student-distrib/boot.S
```

### 3.3 Programmable interrupt controller

## 4 Device Drivers
### 4.1 Keyboard Driver
### 4.2 Real Time Clock Driver
### 4.3 Terminal Driver
### 4.4 Programmable Interval Timer Driver
### 4.5 File System Driver
### 4.6 VGA Text-Mode Driver

## 5 Virtual Memory 

## 6 Process Management 
### 6.1 Process Control Block
### 6.2 ``fork`` 
### 6.3 ``exit``
### 6.4 ``execute`` && ``execv``
### 6.5 Linux Completely Fair Scheduler (CFS)

## 7 Memory Management
### 7.1 Dynamic Memory Allocation
### 7.2 Process Address Space
### 7.3 Demand Paging
### 7.4 Copy On Write

## 8 System Calls

## 9 C Standard Library

## 10 LEGAL
Permission to use, copy, modify, and distribute this software and its
documentation for any purpose, without fee, and without written agreement is
hereby granted, provided that the above copyright notice and the following
two paragraphs appear in all copies of this software.

IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO
ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE

PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE,
SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
