#include <drivers/fs.h>
#include <drivers/rtc.h>
#include <pro/process.h>
#include <vfs/vfs.h>
#include <kmalloc.h>
#include <errno.h>
#include <access.h>
#include <lib.h>

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


static int32_t validate_fd(int32_t fd, thread_t *curr);
static int32_t validate_fname(const int8_t *filename);

/**
 * @brief open a file
 * 
 * @param filename : A file name
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
int32_t do_open(const int8_t *filename) {
   int32_t errno;

   /* validate file descriptor */
   if ((errno = validate_fname(filename)) < 0)
      return errno;

   /* copy data from user space to kernel space */
   // if ((errno = copy_from_user((void *)kbuf, (void *)filename, 
   //                            strlen(filename))) <= 0)
   if (!filename || !*filename)
      return -1;
   if (*filename == '.') 
      return directory_open(filename);
   if (!strcmp(filename, "rtc")) 
      return rtc_open(filename);
   return file_open(filename);
}


/**
 * @brief close a file
 * 
 * @param fd : The file descriptor of the file we want to close
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
int32_t do_close(int32_t fd) {
   thread_t *curr;
   int32_t errno;
   file_op f_op;

   GETPRO(curr);

   /* validate file descriptor */
   if ((errno = validate_fd(fd, curr)) < 0)
      return errno;
   

   /* invoke close routine */
   f_op = curr->fds->fd[fd].f_op;
   return f_op.close(fd); 
}



/**
 * @brief read a file
 * 
 * @param fd : The file descriptor of the file we want to read.
 * @param buf : A buffer array that copys the content from the file
 * @param nbytes The number of bytes to read from the file
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
int32_t do_read(int32_t fd, void *buf, uint32_t nbytes) {
   int32_t errno;
   file_op f_op;
   thread_t *curr;

   GETPRO(curr);

   /* validate file descriptor */
   if ((errno = validate_fd(fd, curr)) < 0)
      return errno;

   /* Might be unsuitable for reading: return -EINVAL in the future */
   
   /* validate nbytes */
   if (nbytes < 0) return -1;

   /* copy data from user space to kernel space */
   // if ((errno = copy_from_user((void *)kbuf, (void *)buf, nbytes)) <= 0)
   //    return errno;
   
   /* copy data from user space to kernel space*/
   /* invoke read routine */
   f_op = curr->fds->fd[fd].f_op;
   return f_op.read(fd, (void *)buf, nbytes);
}


/**
 * @brief write a file
 * 
 * @param fd : The file descriptor of the file we want to write
 * @param buf : A buffer array that copys the content to the file
 * @param nbytes The number of bytes to write to the file *
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
int32_t do_write(int32_t fd, const void *buf, uint32_t nbytes) {
   int32_t errno;
   file_op f_op;
   thread_t *curr;

   GETPRO(curr);

   /* validate file descriptor */
   if ((errno = validate_fd(fd, curr)) < 0)
      return errno;

   /* Might be unsuitable for writing: return -EINVAL in the future */

   /* validate nbytes */
   if (nbytes < 0) return -1;

   /* copy data from user space to kernel space */
   // if ((errno = copy_from_user((void *)kbuf, (void *)buf, nbytes)) <= 0)
   //       return errno;

   /* invoke write routine */
   f_op = curr->fds->fd[fd].f_op;
   return f_op.write(fd, (void *)buf, nbytes);
}



/**
 * @brief Validate a file descriptor
 * 
 * @param fd : a file descriptor
 * @return int32_t : 0 denote success, negative values denote an error condition
 */
static int32_t validate_fd(int32_t fd, thread_t *curr) {
   int max_fd = curr->fds->max_fd;

   if (fd < 0 || fd >= max_fd) return -1;

   if (!curr->fds->fd[fd].f_count) return -1;

   /* should check if the file has permission to access in the future */

   return 0;
}


/**
 * @brief Validate a file name
 * 
 * @param fd : a file name
 * @return int32_t : 0 denote success, negative values denote an error condition
 */
static int32_t validate_fname(const int8_t *filename) {
   if (!filename) return -1;

   if (strlen(filename) > NAMESIZE) return -1;

   return 0;
}


/**
 * @brief Initialize the virtual file system.
 * @param p init the fd for this process
 * 
 * @return int32_t : 0 on success, otherwise on failure.
 */
int32_t fd_init(thread_t *curr) {
    int i;
    
    curr->fds = kmalloc(sizeof(files));
    
    curr->fds->count = 0;
    curr->fds->max_fd = OPEN_MAX;

    for (i = 0; i < OPEN_MAX; ++i) {
        curr->fds->fd[i].f_count = 0;
    }

    return (__open(0, "stdin", TERMINAL, &terminal_op, curr)) + (__open(1, "stdout", TERMINAL, &terminal_op, curr));
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

    thread_t *curr;

    GETPRO(curr);
    
    /* Call read_dentry_by_name to get a new dentry */
    if (!(fd = read_dentry_by_name(fname, &dentry))) { 
        /* Initialize the current file object. */
        fd = file_init(2, &file, &dentry, &f_op, curr); 

        /* Copy the file object into the vfs fd. */
        memcpy((void*)&(curr->fds->fd[fd]), (void*)&file, sizeof(file_t));
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
    thread_t *curr;
    GETPRO(curr);

    if (!curr->fds->fd[fd].f_count) {
        return -1;
    }

    /* Close the file. */
    curr->fds->fd[fd].f_count--; 
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
    thread_t *curr;
    int32_t nread;

    GETPRO(curr);

    file_t *file = &(curr->fds->fd[fd]);
    if (!file->f_count) {
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
    thread_t *curr;
    GETPRO(curr);
    return __open(2, fname, DIRECTORY, &dir_op, curr);
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
    thread_t *curr;

    GETPRO(curr);

    if (!curr->fds->fd[fd].f_count) {
        return -1;
    }

    if (curr->fds->fd[fd].f_pos >= fs->boot->n_dir) 
        return 0;
    if (nbytes > NAMESIZE)
        nread = NAMESIZE;
    else
        nread = nbytes;
    memcpy(buf, (void*)(fs->boot->dirs[curr->fds->fd[fd].f_pos++].fname), nread);
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
int32_t __open(int32_t fd, const int8_t *fname, file_type_t type, file_op *op, thread_t *curr) {
    file_t file;
    dentry_t dentry; 

    memset((void*)&dentry, 0, sizeof(dentry));
    memcpy((void*)(&dentry.fname), (void*)fname, NAMESIZE);
    dentry.inode = 0;   /* ignored here. */
    dentry.type = type;

    if ((fd = file_init(fd, &file, &dentry, op, curr)) < 0) {
        return -1;
    }
    memcpy((void*)&(curr->fds->fd[fd]), (void*)&file, sizeof(file_t));
    return fd;
}



/**
 * @brief copy file descriptor only when a child process first accessing its
 * file descriptors
 * 
 */
void fdcopy(void) {
    thread_t *curr;
    GETPRO(curr);

    /* copy file descriptor when it first tried to open a file */
    if (!curr->fds) {
        curr->fds = kmalloc(sizeof(files));
        curr->fds->count = 0;
        curr->fds->max_fd = OPEN_MAX;
        memcpy((void*)curr->fds, (void*)curr->parent->fds, sizeof(files));
    }
}
