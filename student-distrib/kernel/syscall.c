#include <boot/syscall.h>
#include <pro/process.h>
#include <vfs/vfs.h>
#include <pro/sched.h>
#include <errno.h>
#include <boot/x86_desc.h>
#include <boot/page.h>
#include <lib.h>
#include <io.h>

/**
 * @brief A system call service routine for exiting a process
 * The calling convation of this function is to use the
 * arguments from the stack
 *
 * @param status :
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
asmlinkage void sys_exit(uint8_t status) {
    cli();
    
    /* never returns to the halting process */
    do_exit((uint32_t)status);
}

/**
 * @brief A system call service routine for creating a process
 * The calling convation of this function is to use the
 * arguments from the stack
 *
 * @param status :
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
asmlinkage int32_t sys_fork(void) {
    pid_t pid;
    thread_t *curr, *child;
    uint32_t stack;

    cli();

    /* get current process */
    GETPRO(curr);

    /* get pid of child */
    pid = do_fork(curr, 0);

    /* get child thread */
    child = curr->children[curr->n_children-1];

    /* copy ebp from parent to child */
    asm volatile("movl %%ebp, %0"
                :
                : "m"(child->context->ebp)       
                : "memory" 
    );
    
    /* copy eip from parent to child */
    child->context->eip = *(((uint32_t*)(child->context->ebp)) + 1);

    stack = (get_esp0(curr) - (child->context->ebp) - 8);

    /* copy esp from parent to child */
    child->context->esp = get_esp0(child) - stack;
    
    sti();
        
    return pid;
}


asmlinkage int32_t sys_execve(const int8_t *pathname, int8_t *const argv[]) {
    thread_t *curr;
    int32_t status;

    cli();
    GETPRO(curr);

    status = do_execve(curr, pathname, argv);
    sti();

    return status;
}


asmlinkage int32_t sys_getpid(void) {
    thread_t *t;
    GETPRO(t);
    return t->pid;
}



asmlinkage int32_t sys_getppid(void) {
    thread_t *t;
    GETPRO(t);
    return t->parent->pid;
}   


/**
 * @brief A system call service routine for creating a process
 * The calling convation of this function is to use the
 * arguments from the stack
 *
 * @param cmd : A string contains the command of the process
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
asmlinkage int32_t sys_execute(const int8_t *cmd) {
    thread_t *curr;
    thread_t *child;
    int32_t status;

    cli();

    GETPRO(curr);

    status = do_execute(curr, cmd);
    if (status < 0) 
        return status;

    child = curr->children[curr->n_children-1];
    consoles[child->console_id]->task = child;
    sched_sleep(curr);

    GETPRO(curr);
    child = curr->children[curr->n_children-1];
    child->state = EXITED;
    process_free(child);

    sti();
    
    return curr->context->eax;
}

/**
 * @brief A system call service routine for opening a file
 * The calling convation of this function is to use the
 * arguments from the stack
 *
 * @param filename : A file name
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
asmlinkage int32_t sys_open(const int8_t *filename) {
    fdcopy();
    return do_open(filename);
}

/**
 * @brief A system call service routine for closing a file
 * The calling convation of this function is to use the
 * arguments from the stack
 *
 * @param fd : The file descriptor of the file we want to close
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
asmlinkage int32_t sys_close(int32_t fd) {
    fdcopy();
    return do_close(fd);
}

/**
 * @brief A system call service routine for reading a file
 * The calling convation of this function is to use the
 * arguments from the stack
 *
 * @param fd : The file descriptor of the file we want to read.
 * @param buf : A buffer array that copys the content from the file
 * @param nbytes The number of bytes to read from the file
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
asmlinkage int32_t sys_read(int32_t fd, void *buf, uint32_t nbytes) {
    fdcopy();
    return do_read(fd, buf, nbytes);
}

/**
 * @brief A system call service routine for writing a file
 * The calling convation of this function is to use the
 * arguments from the stack
 *
 * @param fd : The file descriptor of the file we want to write
 * @param buf : A buffer array that copys the content to the file
 * @param nbytes The number of bytes to write to the file *
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
asmlinkage int32_t sys_write(int32_t fd, const void *buf, uint32_t nbytes) {
    fdcopy();
    return do_write(fd, buf, nbytes);
}

/**
 * @brief A system call service routine for copy process argument into buf
 * The calling convation of this function is to use the
 * arguments from the stack
 * @param buf : a buffer from user
 * @param nbytes : number of bytes to copy
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
asmlinkage int32_t sys_getargs(uint8_t *buf, int32_t nbytes) {
    thread_t *curr;

    GETPRO(curr);

    /* no arguments */
    if (curr->argc <= 1)
        return -1;

    /* buf is NULL */
    if (!buf)
        return -1;

    strncpy((char *)buf, curr->argv[1], nbytes);
    return 0;
}

/**
 * @brief A system call service routine for mapping the text-mode video memory into user
 * space at a pre-set virtual address.
 *
 * The calling convation of this function is to use the
 * arguments from the stack
 *
 * @param screen_start : starting screen address
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
asmlinkage int32_t sys_vidmap(uint8_t **screen_start) {
    return do_vidmap(screen_start);
}

asmlinkage int32_t sys_restart(void) {
    return -1;
}

asmlinkage int32_t sys_set_handler(int32_t signum, void *handler_addr) {
    return -1;
}

asmlinkage int32_t sys_sigreturn(void) {
    return -1;
}



/**
 * @brief A system call service routine for dynamic heap allocation in user space, 
 * which change the location of the program break, which de‐fines the end of the 
 * process's data segment.
 *
 * The calling convation of this function is to use the
 * arguments from the stack
 *
 * @param size : size of allocation
 * @return void* : Non-NULL value denote success, NULL denote an error condition
 */
asmlinkage void *sys_sbrk(uint32_t size) {
    thread_t *curr;
    vm_area_t* heap = curr->vm.map_list;
    char* rtn = NULL;
    
    GETPRO(curr);
    while (heap != 0) {
        if (heap->vmflag & VM_HEAP) {
            if (vmalloc(heap, size, PTE_RW | PTE_US) == -1)
                return rtn;
            rtn = curr->vm.brk;
            curr->vm.brk += ((size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
            show_mmap(&curr->vm);
            return rtn;
        }
        heap = heap->next;
    }
    return NULL;
}



/**
 * @brief A system call service routine for creating a new mapping in the virtual 
 * address space of the calling process.
 *
 * The calling convation of this function is to use the
 * arguments from the stack
 *
 * @param addr : virtual address spaces to be mapped
 * @param size : size of allocation
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
asmlinkage int32_t sys_mmap(void *addr, uint32_t size) {
    // if(addr < KERNEL_PAGES * PAGE_SIZE_4MB)
    //     return -1;
    // vm_area_t* area;
    // area->vmflag = VM_WRITE | VM_READ;
    // area->vmend = area->vmstart = (uint32_t)addr;
    
    // vmalloc(area->vmstart, )
    return 0;
}


/**
 * @brief A system call service routine for removing a old mapping in the virtual 
 * address space of the calling process.
 *
 * The calling convation of this function is to use the
 * arguments from the stack
 *
 * @param addr : virtual address spaces to be unmapped
 * @param size : size of allocation
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
asmlinkage int32_t sys_munmap(void *addr, uint32_t size) {
    return 0;
}



asmlinkage int32_t sys_wait(int *wstatus) {
    return 0;
}

asmlinkage int32_t sys_waitpid(pid_t pid, int *wstatus) {
    return 0;
}

