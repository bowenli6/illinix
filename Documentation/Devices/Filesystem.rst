=================================================
Filesystem
=================================================

-------------------
Description
-------------------
The file system memory is divided into 4 kBblocks. The first block is called the boot block, and holds both file system statistics and the directory entries. Both
the statistics and each directory entry occupy 64B, so the file system can hold up to 63 files. The first directory entry
always refers to the directory itself, and is named '.', so it can really hold only 62 files.

Each directory entry gives a name (up to 32 characters, zero-padded, but not necessarily including a terminal EOS
or 0-byte), a file type, and an index node number for the file. File types are 0 for a file giving user-level access to
the real-time clock (RTC), 1 for the directory, and 2 for a regular file. The index node number is only meaningful for
regular files and should be ignored for the RTC and directory types.

Each regular file is described by an index node that specifies the file’s size in bytes and the data blocks that make up
the file. Each block contains 4 kB; only those blocks necessary to contain the specified size need be valid, so be careful
not to read and make use of block numbers that lie beyond those necessary to contain the file data.

The three routines provided by the file system module return -1 on failure, indicating a non-existent file or invalid
index in the case of the first two calls, or an invalid inode number in the case of the last routine. Note that the directory
entries are indexed starting with 0. Also note that the read data call can only check that the given inode is within the
valid range. It does not check that the inode actually corresponds to a file (not all inodes are used). However, if a bad
data block number is found within the file bounds of the given inode, the function should also return -1.

When successful, the first two calls fill in the dentry t block passed as their second argument with the file name, file
type, and inode number for the file, then return 0. The last routine works much like the read system call, reading up to
length bytes starting from position offset in the file with inode number inode and returning the number of bytes
read and placed in the buffer. A return value of 0 thus indicates that the end of the file has been reached.

--------------------
Source Code
--------------------
student-distrib/include/drivers/fs.h

student-distrib/drivers/fs.c
