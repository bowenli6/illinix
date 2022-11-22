=================================================
System Call
=================================================

-------------------
Description
-------------------

-------------
Halt
-------------

The halt system call terminates a process, returning the specified value to its parent process. The system call handler
itself is responsible for expanding the 8-bit argument from BL into the 32-bit return value to the parent program’s
execute system call. Be careful not to return all 32 bits from EBX. This call should never return to the caller.

API: 

int halt (unsigned char status);

System call: 

int32_t sys_halt(uint8_t status);

Service routine: (pro/process.h) 

int32_t do_halt(uint32_t status);


--------------
execute
--------------

The execute system call attempts to load and execute a new program, handing off the processor to the new program
until it terminates. The command is a space-separated sequence of words. The first word is the file name of the
program to be executed, and the rest of the command—stripped of leading spaces—should be provided to the new
program on request via the getargs system call. The execute call returns -1 if the command cannot be executed,
for example, if the program does not exist or the filename specified is not an executable, 256 if the program dies by an
exception, or a value in the range 0 to 255 if the program executes a halt system call, in which case the value returned
is that given by the program’s call to halt.

API:

int execute(const char *cmd);

System call:

int32_t sys_execute(const int8_t *cmd);

Service routine: (kernel/process.c) 

int32_t do_execute(const int8_t *cmd);


--------------
open
--------------

The open system call provides access to the file system. The call should find the directory entry corresponding to the
named file, allocate an unused file descriptor, and set up any data necessary to handle the given type of file (directory,
RTC device, or regular file). If the named file does not exist or no descriptors are free, the call returns -1.

API:

int open(char *filename)

System call:

int32_t sys_open(const int8_t *filename);

Service routine: (kernel/vfs.c) 

int32_t do_open(const int8_t *filename);

--------------
close
--------------

The close system call closes the specified file descriptor and makes it available for return from later calls to open.
You should not allow the user to close the default descriptors (0 for input and 1 for output). Trying to close an invalid
descriptor should result in a return value of -1; successful closes should return 0.

API:

int close(int fd)

System call:

int32_t sys_close(int32_t fd);

Service routine: (kernel/vfs.c) 

int32_t do_close(int32_t fd);


--------------
read
--------------

The read system call reads data from the keyboard, a file, device (RTC), or directory. This call returns the number
of bytes read. If the initial file position is at or beyond the end of file, 0 shall be returned (for normal files and the
directory). In the case of the keyboard, read should return data from one line that has been terminated by pressing
Enter, or as much as fits in the buffer from one such line. The line returned should include the line feed character.
In the case of a file, data should be read to the end of the file or the end of the buffer provided, whichever occurs
sooner. In the case of reads to the directory, only the filename should be provided (as much as fits, or all 32 bytes), and
subsequent reads should read from successive directory entries until the last is reached, at which point read should
repeatedly return 0. For the real-time clock (RTC), this call should always return 0, but only after an interrupt has
occurred (set a flag and wait until the interrupt handler clears it, then return 0). You should use a jump table referenced
by the task’s file array to call from a generic handler for this call into a file-type-specific function. This jump table
should be inserted into the file array on the open system call (see below).

API:

int read(int fd, void *buf, unsigned int nbytes);

System call:

int32_t sys_read(int32_t fd, void *buf, uint32_t nbytes);

Service routine: (kernel/vfs.c) 

int32_t do_read(int32_t fd, void *buf, uint32_t nbytes);


--------------
write
--------------

The write system call writes data to the terminal or to a device (RTC). In the case of the terminal, all data should
be displayed to the screen immediately. In the case of the RTC, the system call should always accept only a 4-byte
integer specifying the interrupt rate in Hz, and should set the rate of periodic interrupts accordingly. Writes to regular
files should always return -1 to indicate failure since the file system is read-only. The call returns the number of bytes
written, or -1 on failure.

API:

int write(int fd, const void *buf, unsigned int nbytes);

System call:

int32_t sys_write(int32_t fd, const void *buf, uint32_t nbytes);

Service routine: (kernel/vfs.c) 

int32_t do_write(int32_t fd, const void *buf, uint32_t nbytes);


--------------
getargs
--------------

The getargs call reads the program’s command line arguments into a user-level buffer. Obviously, these arguments
must be stored as part of the task data when a new program is loaded. Here they are merely copied into user space. If
there are no arguments, or if the arguments and a terminal NULL (0-byte) do not fit in the buffer, simply return -1. The
shell does not request arguments, but you should probably still initialize the shell task’s argument data to the empty
string.

int getargs(unisigned char *buf, int nbytes);

System call:

int32_t sys_getargs(uint8_t *buf, int32_t nbytes);

Service routine: (kernel/syscall.c) 

int32_t sys_getargs(uint8_t *buf, int32_t nbytes);

--------------
vidmap
--------------

The vidmap call maps the text-mode video memory into user space at a pre-set virtual address. Although the address
returned is always the same (see the memory map section later in this handout), it should be written into the memory
location provided by the caller (which must be checked for validity). If the location is invalid, the call should return -1.
To avoid adding kernel-side exception handling for this sort of check, you can simply check whether the address falls
within the address range covered by the single user-level page. Note that the video memory will require you to add
another page mapping for the program, in this case a 4 kB page. It is not ok to simply change the permissions of the
video page located < 4MB and pass that address.

int vidmap(unsigned char **screen_start);

System call:

int32_t sys_vidmap(uint8_t **screen_start);

Service routine: (kernel/page.c) 

int32_t do_vidmap(uint8_t **screen_start);

