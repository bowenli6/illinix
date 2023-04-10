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

Header files: 
```
student-distrib/include/drivers/fs.h
```

Source files:
```
student-distrib/drivers/fs.c
```

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

Header files: 
```
student-distrib/include/drivers/vga.h
student-distrib/include/lib.h
```

Source files:
```
student-distrib/drivers/io.c
student-distrib/lib/io.c
```

## 5 Virtual Filesystem ##
Each task can have up to 8 open files. These open files are represented with a file array, stored in the process control block (PCB). The integer index into this array is called a file descriptor, and this integer is how user-level programs identify the open file.

This array should store a structure containing:
1. The file operations jump table associated with the correct file type. This jump table should contain entries for open, read, write, and close to perform type-specific actions for each operation. open is used for performing type-specific initialization. For example, if we just open’d the RTC, the jump table pointer in this structure should store the RTC’s file operations table.

2. The inode number for this file. This is only valid for data files, and should be 0 for directories and the RTC device file.

3. A “file position” member that keeps track of where the user is currently reading from in the file. Every read system call should update this member.

4. A “flags” member for, among other things, marking this file descriptor as “in-use.”

<img width="493" alt="Screen Shot 2023-04-10 at 10 56 33" src="https://user-images.githubusercontent.com/58064743/230940476-c4d533ce-dbf9-4350-9cf0-b7882af84cc3.png">

When a process is started, the kernel should automatically open stdin and stdout, which correspond to file descrip- tors 0 and 1, respectively. stdin is a read-only file which corresponds to keyboard input. stdout is a write-only file corresponding to terminal output. “Opening” these files consists of storing appropriate jump tables in these two locations in the file array, and marking the files as in-use. For the remaining six file descriptors available, an entry in the file array is dynamically associated with the file being open’d whenever the open system call is made (return -1 if the array is full).

Header files: 
```
student-distrib/include/vfs/vfs.h
student-distrib/include/vfs/file.h
```

Source files:
```
student-distrib/kernel/vfs.c
student-distrib/kernel/file.c
```

## 6 Virtual Memory 

## 7 Process Management 

The process manager is used for virtualizing the CPU. By running one
process, then stopping it and running another, and so forth, the OS can
promote the illusion that many virtual CPUs exist when in fact there is
only one physical CPU (or a few). This basic technique, known as time
sharing of the CPU, allows users to run as many concurrent processes as
they would like; the potential cost is performance, as each will run more
slowly if the CPU(s) must be shared.

Header files: 
```
student-distrib/include/pro/pid.h
student-distrib/include/pro/process.h
```

Source files:
```
student-distrib/kernel/pid.c
student-distrib/kernel/process.c
```

### 7.1 Process Control Block

In illinix, each process uses 8KB memory, for both the kernel stack and the process descriptor entry.

The structure called ``process_t``

```
/* two 4 KB pages containing both the process descriptor and the kernel stack. */
typedef union {
    thread_t thread;
    uint32_t stack[KSTACK_SIZE];
} process_t;
```

The PCB and then be split up to two different parts. The first part is the process descriptor entry for this process (``thread_t thread``) and the remaining part is the kernel stack.

Each process descriptor entry has this structure:

```
/* define a thread that run as a process */
typedef struct thread {
    list_head          task_node;       /* a list of all tasks  */
    list_head          wait_node;       /* a list of all sleeping tasks */
    list_head          run_node;        /* a list of all runnable tasks */
    volatile uint32_t  count;           /* time slice for a task */
    sched_t            sched_info;      /* info used for scheduler */
    volatile pro_state state;	        /* process state */
    volatile uint8_t   flag;            /* process flag */
    int32_t            argc;            /* number of arguments */
    int8_t             **argv;          /* user command line argument */
    pid_t              pid;             /* process id number */
    struct thread      *parent;         /* parent process addr */
    struct thread      **children;      /* child process addr */
    uint64_t           n_children;      /* number of children */
    uint64_t           max_children;    /* max number of children */
    context_t          *context;        /* hardware context */
    uint32_t           usreip;          /* user eip */
    uint32_t           usresp;          /* user esp */
    vmem_t             vm;              /* user virtual memory info */
    files              *fds;            /* opened file descritors */
    uint8_t            kthread;         /* 1 if this thread is belong to the kernel */
    terminal_t         *terminal;       /* terminal for this thread */
    uint32_t           console_id;      /* console for this thread */
    int32_t            nice;            /* nice value */
    uint8_t            **user_vidmap;
} thread_t;
```

### 7.2 ``fork`` 

``fork`` is the most fundamental interface for users to create a new process. The calling process is called the parent process and the created process is called the child process. Child process share the same address space and file descriptors with the parent until either the parent or the child attempts to modify a page, then the memory manager will copy this page and make the new page writable. This technique is called Copy on Write (COW).

The workflows of ``fork``:

```
fork -> sys_fork -> 
 *    do_fork -> process_create
 *            -> process_clone
 *            -> sched_fork -> enqueue_task -> check_preempt_new
 *            -> return child's pid
```

The order of executions of parent and child processes is decided by the scheduler.

### 7.3 ``exit``

When the main function returns, or a process calls ``exit``, this process will be terminated and the reserved resources will be deallocated by the kernel. 

``exit`` will yield the CPU when it returns and let the schedler to decide the next running process.

### 7.4 ``execute`` && ``execv``

```execute``` is the plain way to create a new process without sharing the resource with the parent, the child process will start running the given program immediately.

``execv`` is often called after a ``fork``, to replace the current program image to the new one.

### 7.5 Linux Completely Fair Scheduler (CFS)

CFS stands for "Completely Fair Scheduler," and is the new "desktop" process
scheduler implemented by Ingo Molnar and merged in Linux 2.6.23.  It is the
eplacement for the previous vanilla scheduler's SCHED_OTHER interactivity
code.
 
80% of CFS's design can be summed up in a single sentence: CFS basically models
an "ideal, precise multi-tasking CPU" on real hardware.
"Ideal multi-tasking CPU" is a (non-existent  :-)) CPU that has 100% physical
power and which can run each task at precise equal speed, in parallel, each at
1/nr_running speed.  For example: if there are 2 tasks running, then it runs
each at 50% physical power --- i.e., actually in parallel.

On real hardware, we can run only a single task at once, so we have to
introduce the concept of "virtual runtime."  The virtual runtime of a task
specifies when its next timeslice would start execution on the ideal
multi-tasking CPU described above.  In practice, the virtual runtime of a task
is its actual runtime normalized to the total number of running tasks.

In CFS the virtual runtime is expressed and tracked via the per-task p->se.vruntime (nanosec-unit) value. This way, it's possible to accurately timestamp and measure the "expected CPU time" a task should have gotten.

Small detail: on "ideal" hardware, at any time all tasks would have the same p->se.vruntime value --- i.e., tasks would execute simultaneously and no task would ever get "out of balance" from the "ideal" share of CPU time.
CFS's task picking logic is based on this p->se.vruntime value and it is thus very simple: it always tries to run the task with the smallest p->se.vruntime value (i.e., the task which executed least so far). CFS always tries to split up CPU time between runnable tasks as close to "ideal multitasking hardware" as possible.

Most of the rest of CFS's design just falls out of this really simple concept, with a few add-on embellishments like nice levels, multiprocessing and various algorithm variants to recognize sleepers.

CFS's design is quite radical: it does not use the old data structures for the runqueues, but it uses a time-ordered rbtree to build a "timeline" of future task execution, and thus has no "array switch" artifacts (by which both the previous vanilla scheduler and RSDL/SD are affected).

CFS also maintains the rq->cfs.min_vruntime value, which is a monotonic increasing value tracking the smallest vruntime among all tasks in the runqueue. The total amount of work done by the system is tracked using min_vruntime; that value is used to place newly activated entities on the left side of the tree as much as possible.

The total number of running tasks in the runqueue is accounted through the rq->cfs.load value, which is the sum of the weights of the tasks queued on the runqueue.

CFS maintains a time-ordered rbtree, where all runnable tasks are sorted by the p->se.vruntime key. CFS picks the "leftmost" task from this tree and sticks to it. As the system progresses forwards, the executed tasks are put into the tree more and more to the right --- slowly but surely giving a chance for every task to become the "leftmost task" and thus get on the CPU within a deterministic amount of time.

Summing up, CFS works like this: it runs a task a bit, and when the task schedules (or a scheduler tick happens) the task's CPU usage is "accounted for": the (small) time it just spent using the physical CPU is added to p->se.vruntime. Once p->se.vruntime gets high enough so that another task becomes the "leftmost task" of the time-ordered rbtree it maintains (plus a small amount of "granularity" distance relative to the leftmost task so that we do not over-schedule tasks and trash the cache), then the new leftmost task is picked and the current task is preempted.

Header files: 

```
student-distrib/include/pro/cfs.h
```

Source files:
```
student-distrib/kernel/cfs.c
```

## 8 Memory Management
### 8.1 Dynamic Memory Allocation
### 8.2 Process Address Space
### 8.3 Demand Paging
### 8.4 Copy On Write

## 9 System Calls

## 10 C Standard Library

## 11 LEGAL
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
