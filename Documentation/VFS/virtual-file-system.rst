=================================================
Virtual File System
=================================================

-------------------
Description
-------------------
Each task can have up to 8 open files. These open files are represented with a file array, stored in the process control
block (PCB). The integer index into this array is called a file descriptor, and this integer is how user-level programs
identify the open file.

This array should store a structure containing:

1. The file operations jump table associated with the correct file type. This jump table should contain entries
for open, read, write, and close to perform type-specific actions for each operation. open is used for
performing type-specific initialization. For example, if we just open’d the RTC, the jump table pointer in this
structure should store the RTC’s file operations table.

2. The inode number for this file. This is only valid for data files, and should be 0 for directories and the RTC
device file.

3. A "file position" member that keeps track of where the user is currently reading from in the file. 
Every read system call should update this member.
  
4. A "flags" member for, among other things, marking this file descriptor as "in-use."


--------------------
Source Code
--------------------
student-distrib/include/vfs/ece391_vfs.h

student-distrib/vfs/ece391_vfs.c
