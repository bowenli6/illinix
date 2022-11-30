/**
 * @file process.c
 * @brief Process Management Unit
 * 
 * @fork:
 * fork -> sys_fork -> 
 *    do_fork -> process_create
 *            -> process_clone
 *            -> sched_fork -> enqueue_task -> check_preempt_new
 *            -> return child's pid
 * 
 * @execute:
 * 
 * @exit:
 * 
 * 
 * @version 0.1
 * @date 2022-11-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include <pro/process.h>
#include <boot/page.h>
#include <pro/sched.h>
#include <boot/x86_desc.h>
#include <pro/pid.h>
#include <lib.h>
#include <drivers/fs.h>
#include <kmalloc.h>
#include <access.h>
#include <errno.h> 


thread_t *idle;                 /* process 0 (idle process) */
thread_t *init;                 /* process 1 (init process) */
console_t *console;             /* console contains terminals */
uint32_t ntask;                 /* current number of tasks created */
list_head *task_queue;          /* list of all tasks (idle -> init -> {user task}) */
list_head *wait_queue;          /* list of sleeping tasks (idle -> {sleeping user task || init}) */


/* local helper functions */
static int32_t __exec(thread_t *current, const int8_t *cmd, uint8_t kthread);
static int32_t process_create(thread_t *current, uint8_t kthread);
static int32_t process_clone(thread_t *parent, thread_t *child);
static uint32_t get_eip_user(thread_t *task);
static void process_free(thread_t *current);
static int32_t parse_arg(int8_t *cmd, int8_t *argv[]);
static void switch_to_user(thread_t *curr);
static void console_init(void);
static void update_tss(thread_t *curr);
static void __umap(thread_t *from, thread_t *to);


/**
 * @brief the task of the system process 0
 * 
 */
void swapper(void) { 
    pause();    
}


/**
 * @brief the task of the system process 1
 * 
 */
void init_task(void) {
    /* only execute once during system boot */
    pidmap_init();
    console_init();
    ntask = 0;

    /* the real task of the init process
     * scheduled actively by process 0 */
    while (1) {
        /* init start running here */

        // DO SOMETHING HERE IN THE FUTURE

        /* yield the CPU */
        schedule();
    }
}


/**
 * @brief clone a new process from the current one
 * 
 * @param parent : current process
 * @param kthread : is the new thread a kernel thread?
 * @return int32_t : 0 - to child
 *                 < 0 - error number
 *                 > 0 - pid of the child process
 * Note that fork returns twice: 
 * it returns pid of the child to current process
 * and returns 0 to the child process
 * 
 * pid 0 is reserved by the kernel, which will not be
 * used by the user.
 */
int32_t do_fork(thread_t *parent, uint8_t kthread) {
    thread_t *child;
    uint32_t *eip;
    int32_t errno;

    /* create child process */
    if ((errno = process_create(parent, kthread)) < 0)
        return errno;

    child = parent->children[parent->n_children - 1];

    /* clone thread info of child from parent */
    if (process_clone(parent, child) < 0) {
        process_free(child);
        return -1;
    }
        
    /* set up sched info for child */
    // sched_fork(child); 

    /* save child hardware context */
    save_context(child->context);

    /* child return 0 */
    child->context->eax = 0;

    /* get shared eip in user space */
    child->usreip = get_eip_user(parent);

    /* shared user esp */
    child->usresp = USER_STACK_ADDR;

    ntask++;

    eip = (uint32_t*)(&child->context->eip);
 
    /* set up child's return path */
    asm volatile ("                             \n\
                    movl  $child_ret, (%%eax)   \n\
                    jmp   parent_ret            \n\
                    child_ret:                  \n\
                    pushl %%ecx                 \n\
                    pushl %%edx                 \n\
                    addl  $4, %%esp             \n\
                    call  __umap                \n\
                    call switch_to_user         \n\
                    parent_ret:                 \n\
                  "                               \
                  : "=a" (eip)                    \
                  : "c" (child), "d" (parent)     \
                  : "cc", "memory"   
    );      

    /* map to parent's address space */
    __umap(child, parent);
    // context_switch(parent, child);
    return child->pid;
}



/**
 * @brief clone parent's state into child
 * 
 * @param parent : parent thread
 * @param child : child thread
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
static int32_t process_clone(thread_t *parent, thread_t *child) {
    int i;
    int32_t errno;
    
    /* copy physical memory */
    if ((errno = vmcopy(&child->vm, &parent->vm)) < 0)
        return errno;
    
    /* copy arguments */
    child->argc = parent->argc;
    child->argv = kmalloc(MAXARGS * sizeof(int8_t*));

    for (i = 0; i < MAXARGS; ++i)
        child->argv[i] = kmalloc(ARGSIZE);    

    for (i = 0; i < child->argc; ++i)
        strcpy(child->argv[i], parent->argv[i]);
    
    child->nice = NICE_NORMAL;

    /* child will get real copied when it tries to open a file */
    child->fds = NULL;

    return 0;
}


/**
 * @brief get task's user eip by lookup kernel stack
 * (CPU pushed this value onto kernel stack when switching from user space to kernel space)
 * 
 * @param task : thread info
 * @return uint32_t : task's user eip
 */
static uint32_t get_eip_user(thread_t *task) {
    process_t *process = (process_t *)task;
    uint32_t *stack = (uint32_t*)(&process->stack);
    return stack[STACKEIP];
} 


/**
 * @brief set ebp to parent kernel stack and jump into parent's process
 * (ebp stores the address of esp and eip on stack)
 * 
 * @param status : the status of the exit syscall
 */
void do_exit(uint32_t status) {
    thread_t *child;
    thread_t *parent; 

    GETPRO(child);

    /* check if the current process is running a system thread and it is a shell */
    if ((child->kthread) && (!strcmp(child->argv[0], SHELL))) {
        user_mem_map(child);
        switch_to_user(child);
    }


    /* free the current task */
    parent = child->parent;

    process_free(child);

    ntask--;

    /* leave its children to init */
    // TODO
    
    /* when wait syscall is implement, the parent will get the exit status */
    // sched_exit();

    /* change parent return value to status */
    parent->context->eax = status;

    context_switch(NULL, parent);
}


/**
 * @brief create a new process to execute a program
 * 
 * @param cmd : the program name with arguments
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
int32_t do_execute(const int8_t *cmd) {
    thread_t *parent, *child;  
    uint32_t *eip;
    int32_t errno;

    GETPRO(parent);

    /* call _exec to create the new thread and execute it
     * (only return here when error occurs) */
    if ((errno = __exec(parent, cmd, 0)) < 0) {
        return errno;
    }

    child = parent->children[parent->n_children - 1];

    if (!child->kthread) {
        /* save the kernel stack of the current process */
        save_context(parent->context);
    }

    eip = (uint32_t*)(&parent->context->eip);

    /* set up child's return path */
    asm volatile ("                             \n\
                    movl  $parent, (%%eax)      \n\
                    pushl %%ecx                 \n\
                    call switch_to_user         \n\
                    parent:                     \n\
                  "                               \
                  : "=a" (eip)                    \
                  : "c" (child), "d" (parent)     \
                  : "cc", "memory"   
    );      

    return parent->context->eax;
}


/**
 * @brief low-level operation for executing a program
 * 
 * @param parent: the new thread 
 * @param cmd : command line arguments
 * @param kthread : 1 if it is kernel thread, 0 otherwie
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
static int32_t __exec(thread_t *parent, const int8_t *cmd, uint8_t kthread) {
    int i;
    thread_t *child;
    int32_t errno;
    int32_t argc;
    uint32_t EIP_reg;

    /* arguments array for child */
    int8_t **argv = kmalloc(MAXARGS * sizeof(int8_t*));
    for (i = 0; i < MAXARGS; ++i)
        argv[i] = kmalloc(ARGSIZE);

    /* parse arguments */
    if ((argc = parse_arg((int8_t *)cmd, argv)) < 0) {
        kfree(argv);
        return argc;
    }

    /* create process */
    if ((errno = process_create(parent, kthread)) < 0) {
        kfree(argv);
        return errno;
    }

    /* get child thread */
    child = parent->children[parent->n_children - 1];

    /* get process and set its arguments */
    child->argc = argc;
    child->argv = argv;

    /* map the virtual memory space to to child */
    __umap(parent, child);

    /* executable check and load program image into user's memory */
    if ((errno = pro_loader(argv[0], &EIP_reg, child)) < 0) {
        process_free(child);
        return errno;
    }

    /* init file array */
    if ((errno = fd_init(child)) < 0) {
        process_free(child);
        return errno;
    }
    
    /* create terminals if the program is shell */
    if (!strcmp(argv[0], SHELL))
        child->nice = NICE_SHELL;
    else
        child->nice = NICE_NORMAL;
    

    /* set up sched info */
    // sched_fork(*new);

    // TODO

    /* store registers */
    child->usreip = EIP_reg;
    child->usresp = USER_STACK_ADDR;

    /* increment number of tasks */
    ntask++;
    
    return 0;
}


/**
 * @brief returns the process ID (PID) of the calling process
 * 
 * @return pid_t : The process ID of the calling process
 */
pid_t do_getpid(void) {
    thread_t *t;
    GETPRO(t);
    return t->pid;
}


/**
 * @brief parse the command line arguments 
 * 
 * @param cmd : command line
 * @param argv : array of strings to be set to the arguments
 * 
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
static int32_t parse_arg(int8_t *cmd, int8_t *argv[]) {
    char *delim;                    /* points to the first space delimiter */
    int argc;                       /* number of arguments */
    int8_t buf[strlen(cmd) + 2];    /* buffer that contains command line */

    if (!cmd) return -1;

    strcpy(buf, cmd);
    cmd = buf;

    /* replace trailing \n with space */
    if ((cmd[strlen(cmd) - 1] == '\n') || (cmd[strlen(cmd) - 1] == '\r'))
        cmd[strlen(cmd) - 1] = ' ';
    else
        cmd[strlen(cmd)] = ' ';

    /* skipping leading spaces */
    while (*cmd && (*cmd == ' ')) ++cmd;

    /* build the argv list */
    argc = 0;
    while ((delim = strchr(cmd, ' '))) {
        /* copy argument */
        *delim = '\0';
        strcpy(argv[argc++], cmd);
        cmd = delim + 1;

        /* skipping leading spaces */
        while (*cmd && (*cmd == ' ')) ++cmd;
    }

    /* blank line */
    if (!argc) return -1;
    
    return argc;
}


/**
 * @brief Create a process_t for a new process
 * 
 * @param current : the current thread
 * @param kthread : is the thread a kernel thread?
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
static int32_t process_create(thread_t *current, uint8_t kthread) {
    pid_t pid;
    process_t *p;
    thread_t *t;
    thread_t **children;

    if ((pid = alloc_pid()) < 0) 
        return NULL;

    p = (process_t *)alloc_kstack();
    t = &p->thread;
    
    /* setup current pid */
    t->pid = pid;

    /* set the parent pointer */
    t->parent = current;

    /* allocate memory for context */
    t->context = kmalloc(sizeof(context_t));

    if (!current->children)
        current->children = children_create();
    
    /* check if max_children is full */
    if (current->n_children == current->max_children) {
        children = kmalloc(current->max_children * 2 * sizeof(thread_t*));
        memcpy((void*)children, (void*)current->children, current->max_children * sizeof(thread_t *));
        current->max_children *= 2;
        kfree(current->children);
        current->children = children;
    }   

    /* update parent's children list */
    current->children[current->n_children++] = t;

    t->children = NULL;

    t->n_children = 0;

    t->max_children = MAXCHILDREN;
    
    /* not a kernel thread */
    t->kthread = kthread;

    /* set up terminal for process */
    if (kthread) {
        /* create a new terminal for this kthread */
        t->terminal = terminal_create();
        console->terminals[console->size++] = current->terminal;
    } else {
        /* get the terminal from its parent */
        t->terminal = current->terminal;
    }

    return 0;
}


/**
 * @brief Free a existing process_t
 * 
 * @param current : the process to be freed
 */
static void process_free(thread_t *current) {
    int i;
    thread_t *parent;
    
    if (!current) return;

    parent = current->parent;

    kill_pid(current->pid);
    kfree(current->context);
    kfree(current->fds);

    for (i = 0; i < MAXARGS; ++i)
        kfree(current->argv[i]);
    kfree(current->argv);

    if (current->children) {
        for (i = 0; i < current->max_children; ++i)
            kfree(current->children[i]);
        kfree(current->children);
    }

    __umap(current, current->parent);

    free_kstack((void*)current);

    parent->children[--parent->n_children] = NULL;
    update_tss(parent);
}




/**
 * @brief Perform a context switch to ring 3 (user mode)
 * 
 * (bottom) SS, ESP, EFLAGS, CS, EIP (top)
 * (1) pop and or are for getting IF in EFLAGS
 * so that intrrupts will be enabled in user mode.
 * (2) set USER_DS to the GDT entry for user data segment ds.
 * (3) movl 0 to EAX because child always return 0
 * 
 */
static void switch_to_user(thread_t *curr) {
    
    update_tss(curr);

    sti();

    asm volatile ("                         \n\
                    andl  $0xFF,  %%eax     \n\
                    movw  %%ax,   %%ds      \n\
                    pushl %%eax             \n\
                    pushl %%ebx             \n\
                    pushfl                  \n\
                    popl  %%ebx             \n\
                    orl   $0x200, %%ebx     \n\
                    pushl %%ebx             \n\
                    pushl %%ecx             \n\
                    pushl %%edx             \n\
                    iret                    \n\
                  "
                  :
                  : "a"(USER_DS), "b"(curr->usresp), "c"(USER_CS), "d"(curr->usreip)
                  : "memory"
    );      
}


/**
 * @brief update tss entry for ss0 and esp0
 * 
 * @param _pid : process id
 */
static void update_tss(thread_t *curr) {
    tss.ss0 = KERNEL_DS;
    tss.esp0 = get_esp0(curr);
}


/**
 * @brief get the esp0 of the process
 * 
 * @param curr : thread info
 * @return uint32_t kernel esp
 */
uint32_t get_esp0(thread_t *curr) {
    void *process = (void *)curr;
    return (uint32_t)(process + sizeof(process_t));
}


/**  
 * @brief init console
 * 
 */
static void console_init(void) {
    // int i;
    thread_t *shell;

    /* create console */
    console = kmalloc(sizeof(console_t));    /* never be freed */
    console->terminals = kmalloc(NTERMINAL * sizeof(terminal_t*));
    console->size = 0;

    /* test one shell */
    (void) __exec(init, SHELL, 1);

    shell = init->children[init->n_children - 1];

    terminal_boot = 1;

    /* switch to user mode (ring 3) */
    switch_to_user(shell);
}


/**
 * @brief change process's context from one process to the other
 * 
 * @param from : switch from this process
 * @param to : jump to this process
 */
void context_switch(thread_t *from, thread_t *to) {
    context_t *c1, *c2;

    if (from) 
        c1 = from->context;
    else 
        c1 = NULL;
    
    c2 = to->context;
    swtch(c1, c2);
}


/**
 * @brief create a children list
 * 
 * @return thread_t** : children list
 */
thread_t **children_create(void) {
    int i;
    thread_t **children = kmalloc(MAXCHILDREN * sizeof(thread_t*));
    
    for (i = 0; i < MAXCHILDREN; ++i) {
        children[i] = kmalloc(sizeof(thread_t));
    }
    return children;
}


/**
 * @brief map user virtual memory space
 * 
 * @param from : source process
 * @param to : dest process
 */
static void __umap(thread_t *from, thread_t *to) {
    if (strcmp(from->argv[0], INIT))     /* only unmap if it's not the init process */
        user_mem_unmap(from);
    user_mem_map(to);
}
