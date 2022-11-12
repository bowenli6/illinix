#include <pro/process.h>
#include <pro/sched.h>
#include <boot/syscall.h>
#include <boot/x86_desc.h>
#include <pro/pid.h>
#include <lib.h>
#include <drivers/fs.h>
#include <access.h>
#include <errno.h>  

process_union *init;    /* Process 1 (init process) */
process_union *task_map[TASK_COUNT];    /* Currently process 2 (the shell) and 3 */
pid_t curr_pid;

static void do_halt(uint32_t esp, uint8_t status);
static int32_t process_create(void);
static void process_free(pid_t _pid);
static int32_t context_switch(process_t *p);
static int32_t parse_arg(int8_t* command, int8_t *fname);
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
    (void) sys_execute("shell");
}

static void do_halt(uint32_t esp, uint8_t status) {
    asm volatile ("                         \n\
                    movl %%edx, %%esp       \n\
                    movl %%ebx, %%eax       \n\
                    ret                     \n\
                  "
                  :
                  : "d"(esp), "b"(status)
                  : "cc", "memory"
    );      
}


/* (bottom) SS, ESP, EFLAGS, CS, EIP (top)

 * (1) pop and or are for getting IF in EFLAGS
 * so that intrrupts will be reenabled in user mode.
 * (2) set USER_DS to the GDT entry for user data segment ds.
 * (3) push the addres of 0 in the stack for halt to return
 */
#define SWITCH(esp0, esp, eip, ret)           \
do {                                          \
    asm volatile ("                         \n\
                    pushl $0f               \n\
                    movl  %%esp, %0         \n\
                    pushl $0x2B             \n\
                    pushl %2                \n\
                    pushfl                  \n\
                    popl  %%ebx             \n\
                    orl   $0x200, %%ebx     \n\
                    pushl %%ebx             \n\
                    pushl $0x23             \n\
                    pushl %3                \n\
                    iret                    \n\
                0:                          \n\
                    movl %%eax, %1          \n\
                    "                         \
                    : "=m"(esp0), "=m"(ret)   \
                    : "rm"(esp), "rm"(eip)      \
                    : "cc", "memory"          \
    );                                        \
} while (0)

/**
 * @brief A system call service routine for exiting a process
 * The calling convation of this function is to use the 
 * arguments from the stack
 * 
 * @param status : 
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
asmlinkage int32_t sys_halt(uint8_t status) {
    /* If this is the shell, return to the shell */
    uint32_t esp, ret;

    if (CURRENT->pid == 2) 
        context_switch(CURRENT);
        // SWITCH(esp, CURRENT->esp, CURRENT->eip, ret);

    /* If this is not the shell */
    process_t *curr = CURRENT;
    process_t *parent = CURRENT->parent;
    curr_pid = parent->pid;
    parent->child = NULL;
    update_tss(parent->pid);
    process_free(curr->pid);

    do_halt(parent->esp, status);

    return 0; // never reach here
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
    pid_t pid;
    int32_t ret;
    uint32_t esp;
    process_t *p;
    int32_t errno;
    int8_t fname[NAMESIZE + 1];
    uint32_t EIP_reg;
    memset(fname, 0, NAMESIZE + 1);

    /* parse arguments */
    if ((errno = parse_arg((int8_t *)cmd, fname)) < 0) 
        return errno;

    /* create process */
    if ((pid = process_create()) < 0)
        return pid;

    p = GETPRO(pid);

    /* executable check and load program image into user's memory */
    if ((errno = pro_loader(fname, &EIP_reg, pid)) < 0) {
        process_free(pid);
        return errno;
    }

    /* init file array */
    fd_init(pid);

    p->eip = EIP_reg;
    p->esp = USER_STACK_ADDR;
    p->ebp = USER_STACK_ADDR;
    
    // /* switch to user mode (ring 3) */
    // if (curr_pid) {
    //     curr_pid = p->pid;
    //     /* save the kernel stack of the current process */
    //     SWITCH(CURRENT->parent->esp, CURRENT->esp, CURRENT->eip, ret);
    // } else {
    //     curr_pid = p->pid;
    //     SWITCH(esp, CURRENT->esp, CURRENT->eip, ret);
    // }

    if (curr_pid) {
        /* save the kernel stack of the current process */
        asm volatile("movl %%esp, %0"
                        :
                        : "rm"(esp)               
                        : "cc", "memory" 
                    );
        CURRENT->esp = esp;
    }


    /* switch to user mode (ring 3) */
    context_switch(p);

    return ret;
}

/**
 * @brief returns the process ID (PID) of the calling process
 * 
 * @return pid_t : The process ID of the calling process
 */
pid_t sys_getpid() {
    
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
 * @brief store the entire command line witout the leading file name
 * 
 * @return 0  if the program executes a halt system call
 * @return < 0 if the command cannot be executed
 */
static int32_t parse_arg(int8_t *cmd, int8_t *fname) {
    if (cmd == NULL) {
        return -1;
    }

    /* parse the command into file name and the rest of the command */
    
    while (*cmd) {

        if (*cmd == ' ') {
            *fname = '\0';
            cmd++;
            break;
        }

        if (*cmd == '\n' || *cmd == '\r') {
            *fname = '\0';
            cmd++;
            break;
        }

        *fname++ = *cmd++;
    }

    // while (command[i] != '\0') {
    //     stored_pro[j] = command[i];
    //     i += 1;
    //     j += 1;
    // }
    // stored_pro[j] = '\0';

    return 0;
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
    task_map[_pid-2] = NULL;
    kill_pid();
    free_kstack(_pid-2);
    user_mem_unmap(_pid);
}




/**
 * @brief Perform a context switch to ring 3 (user mode)
 * 
 * @return int32_t : 0 
 */
static int32_t context_switch(process_t *p) {
    
    update_tss(p->pid);

    curr_pid = p->pid;

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
                    0:                      \n\
                  "
                  :
                  : "a"(USER_DS), "b"(p->esp), "c"(USER_CS), "d"(p->eip)
                  : "memory"
    );      
    return 0;
}

static void update_tss(pid_t _pid) {
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KERNEL_STACK_BEGIN - (_pid - 2) * KERNEL_STACK_SZ - 0x4;
}

