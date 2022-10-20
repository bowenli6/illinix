#ifndef _VFS_H
#define _VFS_H

/* A superblock stores information concerning a mounted filesystem. */
typedef struct {

} superblock;


/* A inode stores general information about a specific file. */
/* id : A unique inode number, which uniquely 
 *identifies the file witin the filesystem. */
typedef struct {
    int id;         

} inode;

/* A file stores information about the interaction 
 * between an open file and a process. The information
 * exists only in kernel memory during the period when
 * a process has the file open. */
typedef struct {

} file;

/* A dentry stores information about the linking of a 
 * directory entry (that is, a particular name of the file)
 * with the corresponding file. */
typedef struct {

} dentry;



#endif /* _VFS_H */

  