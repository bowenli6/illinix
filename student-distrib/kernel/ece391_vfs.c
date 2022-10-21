#include <vfs/ece391_vfs.h>
#include <lib.h>
#include <io.h>


ece391_vfs_t vfs;   /* Stores the virtual file system. */

/* Local functions used for opening a file. */

static int32_t __open(int32_t fd, int8_t *fname, file_op *op);


/**
 * @brief Initialize the virtual file system.
 * 
 * @return int32_t : 0 on success, otherwise on failure.
 */
int32_t vfs_init() {
    int i;
    for (i = 0; i < OPEN_MAX; ++i) {
        vfs.fd[i]->f_count = 0;
        vfs.fd[i]->f_flags = UNUSED;
    }
    return (__open(0, "stdin", &terminal_op)) + (__open(1, "stdout", &terminal_op));
}

/**
 * @brief Open the file named fname.
 * 
 * @param fname : A file name.
 * @return int32_t : A file descriptor on success, -1 on failure.
 */
int32_t file_open(const int8_t *fname) {
    int32_t fd;    
    file_t file;
    dentry_t dentry; 
    
    /* Call read_dentry_by_name to get a new dentry */
    if (!(fd = read_dentry_by_name(fname, &dentry))) { 
        /* Initialize the current file object. */
        fd = file_init(2, &file, &dentry, &f_op); 

        /* Copy the file object into the vfs fd. */
        memcpy((void*)vfs.fd[fd], (void*)&file, sizeof(file_t));
    }
    return fd;
}


/**
 * @brief Close the file with file descriptor fd.
 * 
 * @param fd : The file descriptor of the file we want to close.
 * @return int32_t 0 on success, -1 on failure.
 */
int32_t file_close(int32_t fd) {
    if (vfs.fd[fd]->f_flags == UNUSED) {
        puts("ERROR: File does not exist.\n");
        return -1;
    }

    /* Close the file. */
    vfs.fd[fd]->f_count--; 
    vfs.fd[fd]->f_flags = UNUSED;
    return 0;
}


/**
 * @brief Read data from the file and copy them into buf.
 * 
 * @param fd : The file descriptor of the file we want to read.
 * @param buf : A buffer array that copys the content from the file.
 * @param nbytes The number of bytes to read from the file.
 * @return int32_t -1 on failure (non-existent file or invalid inode number), 
 *                    number of bytes read on success.
 */
int32_t file_read(int32_t fd, void *buf, int32_t nbytes) {
    int32_t nread;
    file_t *file = vfs.fd[fd];
    if (file->f_flags == UNUSED) {
        puts("ERROR: File does not exist.\n");
        return -1;
    }

    /* Read data from the file.*/
    if ((nread = read_data(file->f_dentry->inode, file->f_pos, (uint8_t *)buf, nbytes)) != -1) {
        file->f_pos += nread;       /* Update file pointer. */
    }
    return nread;
}


/**
 * @brief Write buf into the file.
 * 
 * @return int32_t : -1 for now.
 */
int32_t file_write(int32_t fd, const void *buf, int32_t nbytes) {
    return -1;
}


/**
 * @brief Open the directory named fname.
 * 
 * @param fname : A directory name.
 * @return int32_t : A file descriptor on success, -1 on failure.
 */
int32_t directory_open(const int8_t *fname) {
    int32_t fd;
    file_t file;
    dentry_t dentry; 
    memcpy((void*)(&dentry.fname), (void*)fname, NAMESIZE);
    dentry.inode = 0;   /* Ignored here. */
    dentry.type = DIRECTORY;
    fd = file_init(2, &file, &dentry, &dir_op);
    memcpy((void*)vfs.fd[fd], (void*)&file, sizeof(file_t));
    return fd;
}


/**
 * @brief Close the directory with file descriptor fd.
 * 
 * @param fd : The file descriptor of the file we want to close.
 * @return int32_t 0 on success, -1 on failure.
 */
int32_t directory_close(int32_t fd) {
    return file_close(fd);
}


/**
 * @brief Read files file name.
 * 
 * @param fd : The file descriptor of the file we want to read.
 * @param buf : A buffer array that copys the content from the file.
 * @param nbytes The number of bytes to read from the file.
 * @return int32_t : number of bytes read on success, -1 on failure.
 */
int32_t directory_read(int32_t fd, void *buf, int32_t nbytes) {
    int32_t nread;
    if (vfs.fd[fd]->f_flags == UNUSED) {
        puts("ERROR: File does not exist.\n");
        return -1;
    }
    if (nbytes > NAMESIZE)
        nread = NAMESIZE;
    else
        nread = nbytes;
    memcpy(buf, (void*)vfs.fd[fd]->f_name, nread);
    return nread;
}


/**
 * @brief Write buf into the file.
 * 
 * @return int32_t : -1 for now.
 */
int32_t directory_write(int32_t fd, const void *buf, int32_t nbytes) {
    return -1;
}



/**
 * @brief Open a file.
 * 
 * @param fd : 0: stdin, 1: stdout
 * @param fname : A file name.
 * @param op : A file opeartion list.
 * @return int32_t : The file descriptor on success, -1 on failure.
 */
static int32_t __open(int32_t fd, int8_t *fname, file_op *op) {
    file_t file;
    dentry_t dentry; 
    memcpy((void*)(&dentry.fname), (void*)fname, NAMESIZE);
    dentry.inode = 0;   /* ignored here. */
    dentry.type = TERMINAL;
    if (file_init(fd, &file, &dentry, &dir_op) != fd) {
        printf("%s allocation error.\n", fname);
        return -1;
    }
    return 0;
}
