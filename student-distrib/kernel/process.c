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
// #include <pro/sched.h>
#include <pro/cfs.h>
#include <drivers/keyboard.h>
#include <boot/x86_desc.h>
#include <pro/pid.h>
#include <lib.h>
#include <drivers/fs.h>
#include <drivers/vga.h>
#include <kmalloc.h>
#include <access.h>
#include <errno.h>


thread_t *idle;                 /* process 0 (idle process) */
thread_t *init;                 /* process 1 (init process) */
uint32_t ntask;                 /* current number of tasks created */
console_t **consoles;           /* array of consoles containing one running task */
console_t *current;             /* current console */
LIST_HEAD(task_queue);          /* list of all tasks (idle -> init -> {user task}) */
LIST_HEAD(wait_queue);          /* list of sleeping tasks (idle -> {sleeping user task || init}) */

/* local helper functions */
static int32_t __exec(thread_t *current, const int8_t *cmd, uint8_t kthread);
static int32_t process_create(thread_t *current, uint8_t kthread);
static int32_t process_clone(thread_t *parent, thread_t *child);
static int32_t parse_arg(int8_t *cmd, int8_t *argv[]);
static inline void switch_to_user(thread_t *curr);
static void console_init(void);
static inline void update_tss(thread_t *curr);
static inline void place_children(thread_t *task);
static inline void overflow_children(thread_t *task);


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
    ntask = 0;
    pidmap_init();
    console_init();

    /* clock starts to tick */
    sti();

    /* the real task of the init process
     * scheduled actively by process 0 */
    while (1) {
        /* init start running here */

        // DO SOMETHING HERE IN THE FUTURE

        /* yield the CPU */
        // sched_tick();
    }
}



/**
 * @brief switch process from prev to next
 * 
 * @param prev : process switch from
 * @param next : process switch to
 */
void inline context_switch(thread_t *prev, thread_t *next) {
    __umap(prev, next);

    if (next != init)
        update_tss(next);

    swtch(prev->context, next->context);
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

    /* set up terminal for process */
    child->terminal = parent->terminal;
        
    /* set up sched info for child */
    sched_fork(child); 
    
    activate_task(child);

    ntask++;  

    child->context->eax = 0;    

    /* map to parent's address space */
    __umap(child, parent);

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
    uint32_t *parent_stack;
    uint32_t *child_stack;
    
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

    /* copy CPU pre-pushed user context
     * stack[....] : general purpose registers 
     * stack[2042] : user eip register
     * stack[2043] : user code segment 
     * stack[2044] : user eflags 
     * stack[2045] : user esp
     * stack[2046] : user data segment
     * stack[....] : hardware reserved
     */
    parent_stack = (uint32_t*)(&((process_t*)parent)->stack);
    child_stack =  (uint32_t*)(&((process_t*)child)->stack);

    child->usreip = parent_stack[USEREIP];
    child->usresp = parent_stack[USERESP];
    
    memcpy((void*)(child_stack + 1024), (void*)(parent_stack + 1024), PAGE_SIZE);

    return 0;
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
    if ((child->kthread) && (!strcmp(child->argv[0], SHELL)))
        switch_to_user(child);
    
    parent = child->parent;

    /* if it has children runnable/sleeping */
    if (child->n_children) {
        /* leave its children to its parent's parnent */
        place_children(child);
    }
    
    ntask--;
    
    /* when wait syscall is implement, the parent will get the exit status */

    parent->context->eax = status;

    consoles[parent->console_id]->task = parent;

    sched_exit(child, parent);
}


/**
 * @brief create a new process to execute a program
 * 
 * @param parent : current process
 * @param cmd : the program name with arguments
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
int32_t do_execute(thread_t *parent, const int8_t *cmd) {
    thread_t *child;  
    int32_t errno;
    
    /* call _exec to create the new thread and execute it
     * (only return here when error occurs) */
    if ((errno = __exec(parent, cmd, 0)) < 0) {
        return errno;
    }

    child = parent->children[parent->n_children - 1];

    /* set up terminal for process */
    child->terminal = parent->terminal;

    child->console_id = parent->console_id;

    /* set up sched info */
    sched_fork(child);
    activate_task(child);

    /* get child esp */
    child->context->esp = get_esp0(child);
    child->context->ebp = child->context->esp;


    /* save current context to child and switch parent to sys_execute
     * when scheduler preempt to child, child will goto line 287 */
    asm volatile("                              \n\
                  movl  $1f,   %[child_eip]     \n\
                  leave                         \n\
                  ret                           \n\
                  1:                            \n\
                  "                                 
                : [child_eip] "=m"(child->context->eip)
                :
                : "memory" 
    );

    GETPRO(child);

    switch_to_user(child);
    

    /* never reach here */
    return 0;
}



/**
 * @brief execute a program at the current process
 * 
 * @param curr : current thread
 * @param pathname : program name
 * @param argv : program arguments
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
int32_t do_execv(thread_t *curr, const int8_t *pathname, int8_t *const argv[]) {
    int i;
    int32_t errno;
    uint32_t EIP_reg;

    curr->argc = 0;

    /* update argument lists */
    for (i = 0; argv[i]; ++i) {
        strcpy((char*)(curr->argv[i]), (char*)(argv[i]));
        if (curr->argv[i][strlen(curr->argv[i]) - 1] == '\n')
            curr->argv[i][strlen(curr->argv[i]) - 1] = '\0';
        curr->argc++;
    } 

    /* set next byte to null */
    if (curr->argc < MAXARGS) curr->argv[curr->argc] = NULL;

    /* executable check and load program image into user's memory */
    if ((errno = pro_loader(curr->argv[0], &EIP_reg, curr)) < 0) {
        return errno;
    }

    /* clear fds */
    if (curr->fds) {
        kfree(curr->fds);
        fd_init(curr);
    }

    /* update nice values */
    if (!strcmp(argv[0], SHELL))
        curr->nice = NICE_SHELL;
    else
        curr->nice = NICE_NORMAL;

    /* store registers */
    curr->usreip = EIP_reg;
    curr->usresp = USER_STACK_ADDR;

    switch_to_user(curr);
    
    return 0;   /* never reach here */
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

    child->argc = argc;
    child->argv = argv;


    /* map the virtual memory space to to child */
    __umap(parent, child);

    /* executable check and load program image into user's memory */
    if ((errno = pro_loader(argv[0], &EIP_reg, child)) < 0) {
        process_free(child);
        return errno;
    }

    /* unmap user space when creating kernel threads */
    if (kthread)
        user_mem_unmap(child);

    /* init file array */
    if ((errno = fd_init(child)) < 0) {
        process_free(child);
        return errno;
    }

    /* store registers */
    child->usreip = EIP_reg;
    child->usresp = USER_STACK_ADDR;

    /* update nice values */
    if (!strcmp(argv[0], SHELL))
        child->nice = NICE_SHELL;
    else
        child->nice = NICE_NORMAL;
    
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
        if (argc == MAXARGS) return -1;
        /* copy argument */
        *delim = '\0';
        strcpy(argv[argc++], cmd);
        cmd = delim + 1;

        /* skipping leading spaces */
        while (*cmd && (*cmd == ' ')) ++cmd;
    }

    /* blank line */
    if (!argc) return -1;

    argv[argc] = NULL;
    
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
    overflow_children(current);

    /* update parent's children list */
    current->children[current->n_children++] = t;

    t->children = NULL;

    t->n_children = 0;

    t->max_children = MAXCHILDREN;
    
    /* not a kernel thread */
    t->kthread = kthread;

    t->state = UNUSED;

    process_vm_init(&t->vm);

    list_add_tail(&t->task_node, &task_queue);

    return 0;
}


/**
 * @brief Free a existing process_t
 * 
 * @param current : the process to be freed
 */
void process_free(thread_t *current) {
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

    if (current->state != EXITED) {
        __umap(current, current->parent);
        update_tss(parent);
    }

    list_del(&current->task_node);

    free_kstack((void*)current);

    parent->children[--parent->n_children] = NULL;
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
static inline void switch_to_user(thread_t *curr) {
    
    update_tss(curr);

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
static inline void update_tss(thread_t *curr) {
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
    return (uint32_t)(process + sizeof(process_t) - 4);
}


/**  
 * @brief init console
 * 
 */
static void console_init(void) {
    int i;
    thread_t *shell;
    console_t *console;
    const uint32_t keys[NTERMINAL] = { F1, F2, F3 };    

    
    /* create consoles */
    for (i = 0; i < NTERMINAL; ++i) {
        (void) __exec(init, SHELL, 1); 
        shell = init->children[i];
        shell->state = UNUSED;
        shell->context->esp = get_esp0(shell);
        shell->context->ebp = shell->context->esp;
        console = kmalloc(sizeof(console_t));
        console->id = i;
        console->fkey = keys[i];
        console->task = shell;
        console->vidmap = (uint8_t*)(VIR_VID_MEM + VIDEO + PAGE_SIZE * i);
        consoles[i] = console;
        shell->console_id = console->id;
        shell->terminal = terminal_create();
        shell->terminal->saved_vidmem = (char*) (VIDEO_BUF_1 + i * PAGE_SIZE);       /* create video memory */
        shell->terminal->vidmem = shell->terminal->saved_vidmem;  /* save back up video memory */
        memset(shell->terminal->saved_vidmem, 0, PAGE_SIZE);
    }

    shell = init->children[0];
    shell->sched_info.on_rq = 1;
    shell->state = RUNNABLE;
    current = consoles[0];
    rq->current = &shell->sched_info;


    /* give the first shell vga memory */
    shell->terminal->vidmem = video_mem;

    sched_fork(shell);
    // activate_task(shell);

    user_mem_map(shell);

    /* console starts */
    terminal_boot = 1;

    /* save current context to shell and switch init to init_task
     * when scheduler preempt init to shell, shell will goto line 629 */
    asm volatile("                              \n\
                  movl  $1f, %[eip1]            \n\
                  movl  $1f, %[eip2]            \n\
                  pushl %[shell]                \n\
                  call  switch_to_user          \n\
                  1:                            \n\
                  "       
                : [eip1] "=m"(init->children[1]->context->eip), [eip2] "=m"(init->children[2]->context->eip)
                : [shell] "rm"(shell)
                : "memory" 
    );
    
    GETPRO(shell);

    switch_to_user(shell);
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
 * @brief place task's children to init
 * 
 * @param task : a thread
 */
static inline void place_children(thread_t *task) {
    int i;
    thread_t *parent_of_parent = task->parent;

    for (i = 0; i < task->n_children; ++i) {
        overflow_children(parent_of_parent);
        parent_of_parent->children[parent_of_parent->n_children++] = task->children[i];
        task->children[i]->parent = parent_of_parent;
    }
}


/**
 * @brief reallocate children list when size reaches max_children
 * 
 * @param task : a thread
 */
static inline void overflow_children(thread_t *task) {
    thread_t **children; 

    if (task->n_children == task->max_children) {
        children = kmalloc(task->max_children * 2 * sizeof(thread_t*));
        memcpy((void*)children, (void*)task->children, task->max_children * sizeof(thread_t *));
        task->max_children *= 2;
        kfree(task->children);
        task->children = children;
    } 
}

