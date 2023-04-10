# Illinix
## 0 WARNING
You SHOULD NOT copy any of source code from this repository into your own ECE 391 project without proper citation. ANY plagiarizing WILL cause a violation of academic integrity.

## 1 Introduction
This project was initially designed as a team project for the ECE 391 Computer Systems Engineering course, with the aim of developing the core of a UNIX-like operating system. The basic requirements for the project included the core subsystems of the OS kernel, including process management, virtual memory abstraction, scheduling, file system, and multiple device drivers. 

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
student-distrib/include/boot/multiboot.h
student-distrib/include/boot/x86_desc.h
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
student-distrib/include/boot/idt.h
student-distrib/include/boot/interrupt.h
student-distrib/include/boot/exception.h
student-distrib/include/boot/syscall.h
student-distrib/include/boot/x86_desc.h
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

The 8259A is a device specifically designed for use
in real time, interrupt driven microcomputer systems.
It manages eight levels or requests and has built-in
features for expandability to other 8259A’s (up to 64
levels). It is programmed by the system’s software
as an I/O peripheral. A selection of priority modes is
available to the programmer so that the manner in
which the requests are processed by the 8259A can
be configured to match his system requirements.
The priority modes can be changed or reconfigured
dynamically at any time during the main program.
This means that the complete interrupt structure can
be defined as required, based on the total system
environment.

There are actually two PICs on most systems, and each has 8 different inputs, plus one output signal that's used to tell the CPU that an IRQ occurred. The slave PIC's output signal is connected to the master PIC's third input (input #2); so when the slave PIC wants to tell the CPU an interrupt occurred it actually tells the master PIC, and the master PIC tells the CPU. This is called "cascade". The master PIC's third input is configured for this and not configured as a normal IRQ, which means that IRQ 2 can't happen.

A device sends a PIC chip an interrupt, and the PIC tells the CPU an interrupt occurred (either directly or indirectly). When the CPU acknowledges the "interrupt occurred" signal, the PIC chip sends the interrupt number (between 00h and FFh, or 0 and 255 decimal) to the CPU. When the system first starts up, IRQs 0 to 7 are set to interrupts 08h to 0Fh, and IRQs 8 to 15 are set to interrupts 70h to 77h. Therefore, for IRQ 6 the PIC would tell the CPU to service INT 0Eh, which presumably has code for interacting with whatever device is connected to the master PIC chip's "input #6". Of course, there can be trouble when two or more devices share an IRQ. Note that interrupts are handled by priority level: 0, 1, 2, 8, 9, 10, 11, 12, 13, 14, 15, 3, 4, 5, 6, 7. So, if IRQ 8 and IRQ 3 come in simultaneously, IRQ 8 is sent to the CPU. When the CPU finishes handling the interrupt, it tells the PIC that it's OK to resume sending interrupts:

Header files: 
```
student-distrib/include/boot/i8259.h
```

Source files:
```
student-distrib/kernel/i8259.c
```

## 4 Device Drivers

Illinix supports multiple different devices and manages them by using the modules and virtual file system. It follows the UNIX design--everything is a file. Each device can be open as a file and abstract to a file descriptor.

### 4.1 Keyboard Driver
The keyboard driver handles keyboard interrupts and monitors each key pressed and released. It keeps incoming input stream from the keyboard device to the input buffer for later usage.

Basically, when a key is pressed, the keyboard controller tells the Programmable Interrupt Controller, or PIC, to cause an interrupt. Because of the wiring of keyboard and PIC, IRQ #1 is the keyboard interrupt, so when a key is pressed, IRQ 1 is sent to the PIC. The role of the PIC will be to decide whether the CPU should be immediately notified of that IRQ or not and to translate the IRQ number into an interrupt vector (i.e. a number between 0 and 255) for the CPU's table.

The OS is supposed to handle the interrupt by talking to the keyboard, via in and out instructions (or inportb/outportb, inportw/outportw, and inportd/outportd in C), asking what key was pressed, doing something about it (such as displaying the key on the screen, and notifying the current application that a key has been pressed), and returning to whatever code was executing when the interrupt came in. Indeed, failure to read the key from the buffer will prevent any subsequent IRQs from the keyboard.

Header files: 
```
student-distrib/include/drivers/keyboard.h
```

Source files:
```
student-distrib/drivers/keyboard.c
student-distrib/drivers/terminal.c
```

### 4.2 Real Time Clock Driver

Real Time Clock is the chip that keeps the system clock up-to-date.

The RTC is capable of multiple frequencies. The base frequency is pre-programmed at 32.768 kHz. It is possible to change this value, but this is the only base frequency that will keep proper time. For this reason, it is strongly recommended that you NOT change the base frequency. The chip also has a "divider" register that will generate other frequencies from the base frequency. The output (interrupt) divider frequency is by default set so that there is an interrupt rate of 1024 Hz. If you need an interrupt frequency other than 1024 Hz, the RTC can theoretically generate 15 interrupt rates between 2 Hz and 32768 Hz. On most machines however, the RTC interrupt rate can not go higher than 8 kHz.

RTC interrupts are disabled by default. If you turn on the RTC interrupts, the RTC will periodically generate IRQ 8.

Header files: 
```
student-distrib/include/drivers/rtc.h
```

Source files:
```
student-distrib/drivers/rtc.c
```

### 4.3 Terminal Driver

When any printable characters are typed at the keyboard, they should be displayed to the screen. This includes handling all alphanumeric characters, symbols, shift and capslock. It keeps track of the screen location for this purpose, and supports vertical scrolling (but not history) and interprets CTRL-L (non-printable key) as meaning “clear the screen and put the cursor at the top” which will make the testing experience more pleasant. It also supports backspace and line-buffered input. The size of the buffer should be 128 characters by default.

The terminal has an external interface to support delivery of external data to the terminal output. In particular, write system calls to the terminal should integrate cleanly with keyboard input.

It is possible to switch between different terminals the ALT+Function-Key combination. We support three terminals, each associated with a different instance of shell. As an example, pressing ALT+F2 while in the first terminal must switch to the active task of the second terminal. Further, it also supports up to six processes in total. For example, each terminal running shell running another program. For the other extreme, have 2 terminals running 1 shell and have 1 terminal running 4 programs (a program on top of shell, on top of shell, etc.).

In order to support the notion of a terminal, there is a separate input buffer associated with each terminal. In addition, each terminal should save the current text screen and cursor position in order to be able to return to the correct state. Switching between terminals is equivalent to switching between the associated active tasks of the terminals. Finally, the keyboard driver must intercept ALT+Function-Key combinations and perform terminal switches.
Lastly, even though a process can be interrupted in either user mode or in kernel mode (while waiting in a system call). After the interrupt, the processor will be in kernel mode, but the data saved onto the stack depends on the state before the interrupt. Each process should have its own kernel stack, but be careful not to implicitly assume either type of transition.

Header files: 
```
student-distrib/include/drivers/terminal.h
```

Source files:
```
student-distrib/drivers/terminal.c
```

### 4.4 Programmable Interval Timer Driver

Besides the Real Time Clock and the Time Stamp Counter, IBM-compatible PCs include another type of time-measuring device called Programmable Interval Timer (PIT ). The role of a PIT is similar to the alarm clock of a microwave oven: it makes the user aware that the cooking time interval has elapsed. Instead of ringing a bell, this device issues a special interrupt called timer interrupt, which notifies the kernel that one more time interval has elapsed.† Another difference from the alarm clock is that the PIT goes on issuing interrupts forever at some fixed frequency established by the kernel. Each IBM-compatible PC includes at least one PIT, which is usually implemented by an 8254 CMOS chip using the 0x40–0x43 I/O ports.

In illinix, the PIT is mostly used for scheduling. Each time the timer interrupt occurs, the scheduler will start running and decide which task should execute next.

Header files: 
```
student-distrib/include/drivers/time.h
```

Source files:
```
student-distrib/drivers/time.c
```

### 4.5 File System Driver

The figure below shows the structure and contents of the file system. The file system memory is divided into 4 kB blocks. The first block is called the boot block, and holds both file system statistics and the directory entries. Both the statistics and each directory entry occupy 64B, so the file system can hold up to 63 files. The first directory entry always refers to the directory itself, and is named “.”, so it can really hold only 62 files.

<img width="484" alt="Screen Shot 2023-04-10 at 10 24 38" src="https://user-images.githubusercontent.com/58064743/230932258-69b50221-81c6-416c-8a6c-b70a4ddcb223.png">

Each directory entry gives a name (up to 32 characters, zero-padded, but not necessarily including a terminal EOS or 0-byte), a file type, and an index node number for the file. File types are 0 for a file giving user-level access to the real-time clock (RTC), 1 for the directory, and 2 for a regular file. The index node number is only meaningful for regular files and should be ignored for the RTC and directory types.

Each regular file is described by an index node that specifies the file’s size in bytes and the data blocks that make up the file. Each block contains 4 kB; only those blocks necessary to contain the specified size need be valid, so be careful not to read and make use of block numbers that lie beyond those necessary to contain the file data.

```
int32 t read dentry by name (const uint8 t* fname, dentry t* dentry);
int32 t read dentry by index (uint32 t index, dentry t* dentry);
int32 t read data (uint32 t inode, uint32 t offset, uint8 t* buf, uint32 t length);
```

The three routines provided by the file system module return -1 on failure, indicating a non-existent file or invalid index in the case of the first two calls, or an invalid inode number in the case of the last routine. Note that the directory entries are indexed starting with 0. Also note that the read data call can only check that the given inode is within the valid range. It does not check that the inode actually corresponds to a file (not all inodes are used). However, if a bad data block number is found within the file bounds of the given inode, the function should also return -1.

When successful, the first two calls fill in the dentry t block passed as their second argument with the file name, file type, and inode number for the file, then return 0. The last routine works much like the read system call, reading up to length bytes starting from position offset in the file with inode number inode and returning the number of bytes read and placed in the buffer. A return value of 0 thus indicates that the end of the file has been reached.

### 4.6 VGA Text-Mode Driver

The most used VGA video mode for a text UI is "VGA mode 3". This is the most commonly used, as it allows direct memory access to a linear address containing each character and its associated attributes. VGA mode 3 provides a text interface 80 characters wide and 25 characters lines per screen, although on modern computers Drawing In a Linear Framebuffer is preferrable, and often mandatory.


In VGA mode 3, the linear text buffer is located in physical at 0xB8000. Reading and writing to and from this address will provide direct manipulation of on screen text. To access a particular character on the screen from X and Y coordinates is simple using the following formula:

``position = (y_position * characters_per_line) + x_position;``

Each character takes up two bytes of space in memory. The first byte is split into two segments, the forecolour, and the backcolour. The second byte is an 8-bit ASCII value of the character to print.


Each character has a colour byte. This colour byte is split up in forecolour and backcolour.

The layout of the byte, using the standard colour palette:

```
Bit 76543210
    ||||||||
    |||||^^^-fore colour
    ||||^----fore colour bright bit
    |^^^-----back colour
    ^--------back colour bright bit OR enables blinking Text
```
Its easy to write to BL, the Colour Nibbles(4Bit), in a Hex Value.

For Example:

```
0x01 sets the background to black and the fore colour to blue

0x10 sets the background to blue and the fore colour to black 

0x11 sets both to blue.
```

Scrolling is achieved through copying the second line of characters onto the first, the third onto the second, etc. Then clearing the last line of text with null characters (usually a blank space with the fore/back colours of 7 and 0, respectively). In text mode 0, this is accomplished by copying characters 80-159 in to memory position 0-79, 160-239 into 80-159, etc.

Without BIOS access, manipulating the cursor requires sending data directly to the hardware.

Enabling the cursor also allows you to set the start and end scanlines, the rows where the cursor starts and ends. The highest scanline is 0 and the lowest scanline is the maximum scanline (usually 15).

```
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
	outb(0x3D4, 0x0A);
	outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);
 
	outb(0x3D4, 0x0B);
	outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}


void disable_cursor()
{
	outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);
}
```

We don't need to update the cursor's location every time a new character is displayed. It would be faster to instead only update it after printing an entire string.

```
void update_cursor(int x, int y)
{
	uint16_t pos = y * VGA_WIDTH + x;
 
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}
```

With this code, you get: ``pos = y * VGA_WIDTH + x``. To obtain the coordinates, just calculate: ``y = pos / VGA_WIDTH; x = pos % VGA_WIDTH;``.

```
uint16_t get_cursor_position(void)
{
    uint16_t pos = 0;
    outb(0x3D4, 0x0F);
    pos |= inb(0x3D5);
    outb(0x3D4, 0x0E);
    pos |= ((uint16_t)inb(0x3D5)) << 8;
    return pos;
}
```

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
