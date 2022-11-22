#include <pro/process.h>
#include <pro/sched.h>
#include <boot/x86_desc.h>
#include <pro/pid.h>
#include <lib.h>
#include <drivers/fs.h>
#include <access.h>
#include <errno.h> 

process_union *init;    /* Process 1 (init process) */
process_union *task_map[TASK_COUNT];    /* Currently process 2 (the shell) and 3 */
pid_t curr_pid;         /* current pid*/


/* local helper functions */

static int32_t process_create(void);
static void process_free(pid_t _pid);
static void context_switch(process_t *p);
static int32_t parse_arg(int8_t *cmd, char argv[MAXARGS][COMMAND_LEN]);
static int32_t copy_args(process_t *p, char argv[MAXARGS][COMMAND_LEN]);
static void update_tss(pid_t _pid);


/**
 * @brief the task of the system process 0
 * working as a scheduler and a time interrupt handler (only for now)
 */
void swapper() { 
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
void init_task() {
    pidmap_init();
    curr_pid = 0;
    (void) do_execute("shell");
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

    process_t *curr = CURRENT;
    process_t *parent = CURRENT->parent;
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
                  : "d"(parent->ebp), "b"(status)
                  : "memory"
    );      
}


/**
 * @brief create a new process to execute a program
 * 
 * @param cmd : the program name with arguments
 */
int32_t do_execute(const int8_t *cmd) {
    pid_t pid;
    uint32_t ebp;
    process_t *p;
    int32_t errno;
    int32_t argc;
    char argv[MAXARGS][COMMAND_LEN];    
    uint32_t EIP_reg;

    /* parse arguments */
    if ((argc = parse_arg((int8_t *)cmd, argv)) < 0) 
        return argc;

    /* create process */
    if ((pid = process_create()) < 0)
        return pid;

    /* get process and set its arguments */
    p = GETPRO(pid);
    p->argc = argc;
    if ((errno = copy_args(p, argv)) < 0) {
        process_free(pid);
        update_tss(CURRENT->pid);
        user_mem_map(CURRENT->pid);
        return errno; 
    }    

    /* executable check and load program image into user's memory */
    if ((errno = pro_loader(argv[0], &EIP_reg, pid)) < 0) {
        process_free(pid);
        update_tss(CURRENT->pid);
        user_mem_map(CURRENT->pid);
        return errno;
    }

    /* init file array */
    fd_init(pid);

    /* store registers */
    p->eip = EIP_reg;
    p->esp = USER_STACK_ADDR;
    p->ebp = USER_STACK_ADDR;

    if (curr_pid) {
        /* save the kernel stack of the current process */
        asm volatile("movl %%ebp, %0"
                    :
                    : "rm"(ebp)             
                    : "memory" 
                    );
        CURRENT->ebp = ebp;
    }

    /* switch to user mode (ring 3) */
    context_switch(p);

    return 0;   /* never reach here */
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
static int32_t copy_args(process_t *p, char argv[MAXARGS][COMMAND_LEN]) {
    int i;

    if (p->argc > MAXARGS) return -1;

    if (!argv) return -1;

    for (i = 0; i < p->argc; ++i) {
        strcpy(p->argv[i], argv[i]);
    }

    return i;
}



/**
 * @brief Create a process_t for a new process
 * 
 * @return int32_t : 0
 */
static int32_t process_create(void) {
    pid_t pid;
    process_t *p;

    if ((pid = alloc_pid()) < 0) 
        return pid;

    task_map[pid-2] = (process_union *)alloc_kstack(pid-2);

    p = GETPRO(pid);
    
    /* setup current pid */
    p->pid = pid;

    /* setup the kernel stack info */

    /* set the parent pointer */
    if (pid == 2) {
        p->parent = NULL;
    } else {
        p->parent = CURRENT;
        CURRENT->child = p;
    }

    return pid;
}


/**
 * @brief Free a existing process_t
 * 
 */
static void process_free(pid_t _pid) {
    CURRENT->child = NULL;
    task_map[_pid-2] = NULL;
    kill_pid();
    free_kstack(_pid-2);
    user_mem_unmap(_pid);
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
static void context_switch(process_t *p) {
    
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
                  : "a"(USER_DS), "b"(p->esp), "c"(USER_CS), "d"(p->eip)
                  : "memory"
    );      
}


/**
 * @brief update tss entry for ss0 and esp0
 * 
 * @param _pid : process id
 */
static void update_tss(pid_t _pid) {
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KERNEL_STACK_BEGIN - (_pid - 1) * KERNEL_STACK_SZ - 0x4;
}

