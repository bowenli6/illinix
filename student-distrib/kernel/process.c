#include <pro/process.h>
#include <pro/sched.h>
#include <boot/x86_desc.h>
#include <pro/pid.h>
#include <lib.h>
#include <drivers/fs.h>
#include <access.h>
#include <errno.h> 


thread_t *init;                 /* process 1 (init process) */
thread_t *tasks[NTASK];         /* list of user processes (pid starting from 2 to PID_SIZE) */
console_t *console;             /* the console */
pid_t curr_pid;                 /* current pid */


/* local helper functions */
static void console_init(void);
static int32_t process_create(uint8_t kthread);
static void process_free(pid_t _pid);
static int32_t parse_arg(int8_t *cmd, char argv[MAXARGS][COMMAND_LEN]);
static int32_t copy_args(thread_t *t, char argv[MAXARGS][COMMAND_LEN]);
static void update_tss(pid_t _pid);
static int32_t __exec(const int8_t *cmd, uint8_t kthread);
static thread_t *get_task(pid_t pid);


/**
 * @brief the task of the system process 0
 * working as a scheduler 
 */
void swapper(void) { 
    /* keep trying to schedule and pause when no process can be scheduled */
    while (!schedule()) 
        pause();
}


/**
 * @brief create a user process wit pid 2 (the shell)
 * 
 * the task of the init process
 * 
 * shell process will be created by fork in the future
 */
void init_task(void) {
    cli();
    init->state = RUNNING;
    pidmap_init();
    console_init();
    
    /* typing user name and password */
    // TODO

    init->state = RUNNABLE;
    save_context(init->context);
    /* adopt orphan processes */
    // TODO
}


/**
 * @brief set ebp to parent kernel stack and jump into parent's process
 * (ebp stores the address of esp and eip on stack)
 * 
 * @param status : the status of the halt syscall
 */
void do_halt(uint32_t status) {
    /* If this is the shell, return to the shell */
    if (CURRENT->pid == 2) {
        process_free(CURRENT->pid);
        init_task();
    }

    thread_t *curr = CURRENT;
    thread_t *parent = CURRENT->parent;
    curr_pid = parent->pid;
    parent->child = NULL;
    update_tss(parent->pid);
    process_free(curr->pid);
    user_mem_map(parent->pid);

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
    thread_t *t;   
    uint32_t ebp;
    int32_t errno;

    if ((errno = __exec(cmd, 0)) < 0)
        return errno;

    if (!t->kthread) {
        /* save the kernel stack of the current process */
        asm volatile("movl %%ebp, %0"
                    :
                    : "rm"(ebp)             
                    : "memory" 
                    );
        CURRENT->context->ebp = ebp;
    }

    /* switch to user mode (ring 3) */
    switch_to_user(t);

    return 0;   /* never reach here */
}


/**
 * @brief lower operation for executing a process
 * 
 * @param cmd : command line arguments
 * @param kthread : 1 if it is kernel thread, 0 otherwie
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
static int32_t __exec(const int8_t *cmd, uint8_t kthread) {
    pid_t pid;
    thread_t *t;
    int32_t errno;
    int32_t argc;
    char argv[MAXARGS][COMMAND_LEN];    
    uint32_t EIP_reg;
    terminal_t *terminal;

    /* parse arguments */
    if ((argc = parse_arg((int8_t *)cmd, argv)) < 0) 
        return argc;

    /* create process */
    if ((pid = process_create(kthread)) < 0)
        return pid;

    /* get process and set its arguments */
    t = GETPRO(pid);
    t->argc = argc;
    if ((errno = copy_args(t, argv)) < 0) {
        process_free(pid);
       
        return errno; 
    }    

    /* executable check and load program image into user's memory */
    if ((errno = pro_loader(argv[0], &EIP_reg, pid)) < 0) {
        process_free(pid);
        return errno;
    }

    /* create terminals if the program is shell */
    if (!strcmp(argv[0], SHELL)) {

        if (console->size == console->max) {
            process_free(pid);
            return -1;
        }

        if (!(terminal = terminal_create(pid))) {
            process_free(pid);
            return -1;
        }

        console->terminals[console->size++] = terminal;
    }

    /* init file array */
    fd_init(pid);

    /* store registers */
    t->eip = EIP_reg;
    t->context->esp = USER_STACK_ADDR;
    t->context->ebp = USER_STACK_ADDR;
    
    return 0;
}


/**
 * @brief returns the process ID (PID) of the calling process
 * 
 * @return pid_t : The process ID of the calling process
 */
pid_t sys_getpid() {
    // TODO
    return 0;
}


/**
 * @brief returns the process ID of the parent of the calling process.
 * This will be either the ID of the process that created this process us‐
 * ing fork(), or, if that process has already terminated, the ID  of  the
 * process  to which this process has been reparented (either init(1) or a
 * "subreaper" process defined via the prctl(2) PR_SET_CHILD_SUBREAPER op‐
 * eration).
 * 
 * @return pid_t : process ID of the parent of the calling process
 */
pid_t sys_getppid() {
    // TODO
    return 0;
}


/**
 * @brief parse the command line arguments 
 * 
 * @param cmd : command line
 * @param argv : array of strings to be set to the arguments
 * 
 * @return number of argumnets (including file name)
 * @return < 0 if the command cannot be executed
 */
static int32_t parse_arg(int8_t *cmd, char argv[MAXARGS][COMMAND_LEN]) {
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
static int32_t copy_args(thread_t *t, char argv[MAXARGS][COMMAND_LEN]) {
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
 * @return int32_t : 0
 */
static int32_t process_create(uint8_t kthread) {
    pid_t pid;
    process_t *p;
    thread_t *t;

    if ((pid = alloc_pid()) < 0) 
        return pid;

<<<<<<< HEAD
    task_map[pid-2] = (process_union *)alloc_kstack();
=======
    /* process overflow */
    if (pid >= NTASK + TASKSTART) 
        return -1;
>>>>>>> 71ad153 (init scheduler module)

    p = (process_t *)alloc_kstack(pid-2);
    tasks[pid-TASKSTART] = &p->thread;

    t = GETPRO(pid);
    
    /* setup current pid */
    t->pid = pid;

    /* setup the kernel stack info */

    /* set the parent pointer */
    if (kthread) {
        t->parent = init;
    } else {
        t->parent = CURRENT;
        CURRENT->child = p;
    }

    /* not a kernel thread */
    t->kthread = kthread;

    return pid;
}


/**
 * @brief Free a existing process_t
 * 
 */
static void process_free(pid_t pid) {
    CURRENT->child = NULL;
<<<<<<< HEAD
    free_kstack(task_map[_pid-2]);
    task_map[_pid-2] = NULL;
    kill_pid();
    user_mem_unmap(_pid);
=======
    tasks[pid-TASKSTART] = NULL;
    kill_pid();
    free_kstack(pid-TASKSTART);
    user_mem_unmap(pid);
    update_tss(CURRENT->pid);
    user_mem_map(CURRENT->pid);
>>>>>>> 71ad153 (init scheduler module)
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

    curr_pid = p->pid;

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
                  : "a"(USER_DS), "b"(p->context->esp), "c"(USER_CS), "d"(p->eip)
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
<<<<<<< HEAD
    tss.esp0 = KERNEL_STACK_BEGIN - (_pid - 1) * KERNEL_STACK_SZ - 0x4;
=======
    tss.esp0 = get_esp0(pid);
>>>>>>> 71ad153 (init scheduler module)
}


/**
 * @brief Get the esp0 of the process
 * 
 * @param pid : process id
 * @return uint32_t kernel esp
 */
uint32_t get_esp0(pid_t pid) {
    return KERNEL_STACK_BEGIN - pid * KERNEL_STACK_SZ - 0x4;
}


/**
 * @brief init console
 * 
 */
static void console_init(void) {
    int num_shell = 3;

    /* create console */
    console = (console_t *)kmalloc(sizeof(console));

    /* init three shells */
    while (num_shell--)
        (void)__exec(SHELL, 1);
}


/**
 * @brief Get the task has process id = pid
 * USE FOR DEBUG ONLY
 * 
 * @param pid : process id
 * @return thread_t : thread object
 */
static thread_t *get_task(pid_t pid) {
    return tasks[pid-TASKSTART];
}
