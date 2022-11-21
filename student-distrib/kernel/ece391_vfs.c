#include <vfs/ece391_vfs.h>
#include <drivers/terminal.h>
#include <drivers/rtc.h>
#include <drivers/fs.h>
#include <boot/syscall.h>
#include <pro/process.h>
#include <errno.h>
#include <lib.h>
#include <io.h>

/* File operation used for regular files. */
static file_op f_op = {
    .open = file_open,
    .close = file_close,
    .read = file_read,
    .write = file_write
};

/* Directory operation used for '.'. */
static file_op dir_op = {
    .open = directory_open,
    .close = directory_close,
    .read = directory_read,
    .write = directory_write
};

/* Terminal operation used for stdin and stdout. */
static file_op terminal_op = {
    .open = terminal_open,
    .close = terminal_close,
    .read = terminal_read,
    .write = terminal_write
};


/**
 * @brief Initialize the virtual file system.
 * @param p init the fd for this process
 * 
 * @return int32_t : 0 on success, otherwise on failure.
 */
int32_t fd_init(pid_t pid) {
    int i;
    process_t *p = GETPRO(pid);

    p->fds.count = 0;
    p->fds.max_fd = OPEN_MAX;

    for (i = 0; i < OPEN_MAX; ++i) {
        p->fds.fd[i].f_count = 0;
        p->fds.fd[i].f_flags = UNUSED;
    }

    terminal_init(pid);
    return (__open(0, "stdin", TERMINAL, &terminal_op, pid)) + (__open(1, "stdout", TERMINAL, &terminal_op, pid));
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
        fd = file_init(2, &file, &dentry, &f_op, CURRENT->pid); 

        /* Copy the file object into the vfs fd. */
        memcpy((void*)&(CURRENT->fds.fd[fd]), (void*)&file, sizeof(file_t));
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
    if (!CURRENT->fds.fd[fd].f_count) {
        return -1;
    }

    /* Close the file. */
    CURRENT->fds.fd[fd].f_count--; 
    CURRENT->fds.fd[fd].f_flags = UNUSED;
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
    file_t *file = &(CURRENT->fds.fd[fd]);
    if (file->f_flags == UNUSED) {
        return -1;
    }

    /* Read data from the file.*/
    if ((nread = read_data(file->f_dentry.inode, file->f_pos, (uint8_t *)buf, nbytes)) != -1) {
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
    return __open(2, fname, DIRECTORY, &dir_op, CURRENT->pid);
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
 * @brief Read directory contents.
 * 
 * @param fd : The file descriptor of the file we want to read.
 * @param buf : A buffer array that copys the content from the file.
 * @param nbytes The number of bytes to read from the file.
 * @return int32_t : number of bytes read on success, -1 on failure.
 */
int32_t directory_read(int32_t fd, void *buf, int32_t nbytes) {
    int32_t nread;
    if (CURRENT->fds.fd[fd].f_flags == UNUSED) {
        return -1;
    }

    if (CURRENT->fds.fd[fd].f_pos >= fs.boot->n_dir) 
        return 0;
    if (nbytes > NAMESIZE)
        nread = NAMESIZE;
    else
        nread = nbytes;
    memcpy(buf, (void*)(fs.boot->dirs[CURRENT->fds.fd[fd].f_pos++].fname), nread);
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
 * @brief Open a file for stdin, stdout, and directory.
 * 
 * @param fd : 0: stdin, 1: stdout, >= 2 for directory.
 * @param fname : A file name.
 * @param op : A file opeartion list.
 * @param type : The file type.
 * @param p : the current process
 * @return int32_t : The file descriptor on success, -1 on failure.
 */
int32_t __open(int32_t fd, const int8_t *fname, file_type_t type, file_op *op, pid_t pid) {
    file_t file;
    dentry_t dentry; 
    process_t *p = GETPRO(pid);
    memset((void*)&dentry, 0, sizeof(dentry));
    memcpy((void*)(&dentry.fname), (void*)fname, NAMESIZE);
    dentry.inode = 0;   /* ignored here. */
    dentry.type = type;
    if ((fd = file_init(fd, &file, &dentry, op, p->pid)) < 0) {
        return -1;
    }
    memcpy((void*)&(p->fds.fd[fd]), (void*)&file, sizeof(file_t));
    return fd;
}
