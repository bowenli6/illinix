#include <pro/process.h>
#include <pro/sched.h>
#include <boot/x86_desc.h>
#include <pro/pid.h>
#include <lib.h>
#include <drivers/fs.h>
#include <access.h>
#include <errno.h> 


thread_t *idle;                 /* process 0 (idle process) */
thread_t *init;                 /* process 1 (init process) */
console_t *console;             /* the console */
uint32_t ntask;                 /* current number of tasks created (does not count sched and init) */
list_head *task_head;           /* the head of the task list */
list_head *wait_queue;          /* list of sleeping tasks */

/* local helper functions */
static int32_t __exec(thread_t *current, thread_t **new, const int8_t *cmd, uint8_t kthread);
static thread_t *process_create(thread_t *current, uint8_t kthread);
static void process_free(thread_t *current, pid_t pid);
static int32_t parse_arg(int8_t *cmd, int8_t *argv[]);
static int32_t copy_args(thread_t *t, int8_t *argv[]);
static void switch_to_user(thread_t *p);
static void console_init(void);
static void update_tss(pid_t _pid);


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
        /* init start running */

        // DO SOMETHING HERE IN THE FUTURE..

        /* yield the CPU to process 0 again */
        schedule();
    }
}


/**
 * @brief set ebp to parent kernel stack and jump into parent's process
 * (ebp stores the address of esp and eip on stack)
 * 
 * @param status : the status of the exit syscall
 */
void do_exit(uint32_t status) {
    thread_t *t;
    thread_t **new; 

    GETPRO(t);
    new = kmalloc(sizeof(thread_t *));


    /* check if the current process is running a system thread */
    if (t->kthread && strcmp(t->argv[0], SHELL)) {
        // TODO       
    }

    /* free the current task */
    thread_t *parent = t->parent;
    parent->child = NULL;
    process_free(parent, t->pid);

    ntask--;


    /* switch to its parent's stack */
    sti();
    asm volatile ("                         \n\
                    movl %%edx, %%ebp       \n\
                    movl %%ebx, %%eax       \n\
                    leave                   \n\
                    ret                     \n\
                  "
                  :
                  : "d"(parent->context->ebp), "b"(status)
                  : "memory"
    );      
}


/**
 * @brief create a new process to execute a program
 * 
 * @param cmd : the program name with arguments
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
int32_t do_execute(const int8_t *cmd) {
    thread_t **new; 
    thread_t *current, *child;  
    int32_t errno;

    GETPRO(current);
    new = kmalloc(sizeof(thread_t *));

    /* call _exec to create the new thread and execute it
     * (only return here when error occurs) */
    if ((errno = __exec(current, new, cmd, 0)) < 0) {
        kfree(new);
        return errno;
    }

    if (!(*new)->kthread) {
        /* save the kernel stack of the current process */
        save_context(current->context);
    }

    
    child = *new;
    kfree(new);

    /* switch to user mode (ring 3) */
    switch_to_user(child);

    return 0;   /* never reach here */
}


/**
 * @brief lower operation for executing a process
 * 
 * @param t: the new thread 
 * @param cmd : command line arguments
 * @param kthread : 1 if it is kernel thread, 0 otherwie
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
static int32_t __exec(thread_t *current, thread_t **new, const int8_t *cmd, uint8_t kthread) {
    pid_t pid;
    int32_t errno;
    int32_t argc;
    uint32_t EIP_reg;
    terminal_t *terminal;
    int8_t **argv = kmalloc(MAXARGS * ARGSIZE);

    /* parse arguments */
    if ((argc = parse_arg((int8_t *)cmd, argv)) < 0)
        return argc;

    /* create process */
    if (!(*new = process_create(current, kthread)))
        return -1;

    /* get process and set its arguments */
    (*new)->argc = argc;

    if ((errno = copy_args(*new, argv)) < 0) {
        process_free(current, pid);
        return errno; 
    }    

    /* executable check and load program image into user's memory */
    if ((errno = pro_loader(argv[0], &EIP_reg, pid)) < 0) {
        process_free(current, pid);
        return errno;
    }
    

    /* create terminals if the program is shell */
    if (!strcmp(argv[0], SHELL)) {
        (*new)->sched_info.nice = -5;

        if (console->size == console->max) {
            process_free(*new, pid);
            return -1;
        }

        if (!(terminal = terminal_create(*new))) {
            process_free(*new, pid);
            return -1;
        }

        console->terminals[console->size++] = terminal;
    } else {
        (*new)->sched_info.nice = 0;
    }

    /* init file array */
    fd_init(*new);

    /* set up sched info */
    set_sched_task(&(*new)->sched_info);

    /* store registers */
    (*new)->usreip = EIP_reg;
    (*new)->usresp = USER_STACK_ADDR;

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
 * @brief copy the argv into p->agrv
 * 
 * @param p : process
 * @param argv : arguments list
 * @return int32_t : number of arguments copied, or < 0 if cannot copy
 */
static int32_t copy_args(thread_t *t, int8_t *argv[]) {
    int i;

    if (t->argc > MAXARGS) return -1;

    if (!argv) return -1;

    for (i = 0; i < t->argc; ++i) {
        strcpy(t->argv[i], argv[i]);
    }

    return i;
}



/**
 * @brief Create a process_t for a new process
 * 
 * @param current : the current thread
 * @param child : the thread to be created
 * @param kthread : is the thread a kernel thread?
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
static thread_t *process_create(thread_t *current, uint8_t kthread) {
    pid_t pid;
    process_t *p;
    thread_t *t;

    if ((pid = alloc_pid()) < 0) 
        return NULL;

    p = (process_t *)alloc_kstack(pid);
    t = &p->thread;
    
    /* setup current pid */
    t->pid = pid;

    /* set the parent pointer */
    t->parent = current;
    current->child = t;
    
    /* not a kernel thread */
    t->kthread = kthread;

    return t;
}


/**
 * @brief Free a existing process_t
 * 
 */
static void process_free(thread_t *current, pid_t pid) {
    kill_pid(pid);
    free_kstack(pid);
    user_mem_unmap(pid);
    update_tss(current->pid);
    user_mem_map(current->pid);
}




/**
 * @brief Perform a context switch to ring 3 (user mode)
 * 
 * (bottom) SS, ESP, EFLAGS, CS, EIP (top)
 * (1) pop and or are for getting IF in EFLAGS
 * so that intrrupts will be enabled in user mode.
 * (2) set USER_DS to the GDT entry for user data segment ds.
 * (3) push the addres of 0 in the stack for halt to return
 * 
 */
void switch_to_user(thread_t *p) {
    
    update_tss(p->pid);

    sti();

    asm volatile ("                         \n\
                    andl  $0xFF, %%eax      \n\
                    movw  %%ax, %%ds        \n\
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
                  : "a"(USER_DS), "b"(p->usresp), "c"(USER_CS), "d"(p->usreip)
                  : "memory"
    );      
}


/**
 * @brief update tss entry for ss0 and esp0
 * 
 * @param _pid : process id
 */
static void update_tss(pid_t pid) {
    tss.ss0 = KERNEL_DS;
    tss.esp0 = get_esp0(pid);
}


/**
 * @brief get the esp0 of the process
 * 
 * @param pid : process id
 * @return uint32_t kernel esp
 */
uint32_t get_esp0(pid_t pid) {
    // return KERNEL_STACK_BEGIN - pid * KERNEL_STACK_SZ - 0x4;
    return 0; // TODO
}


/**
 * @brief init console
 * 
 */
static void console_init(void) {
    int i;
    const int num_shell = 3;
    thread_t **new = kmalloc(sizeof(thread_t *));

    /* create console */
    console = kmalloc(sizeof(console));    /* never be freed */

    /* init three shells */
    for (i = 0; i < num_shell; ++i) {
        (void)__exec(init, new, SHELL, 1);
        console->terminals[i]->shell = *new;
    }

    kfree(new);
}
