#include <pro/process.h>
#include <boot/syscall.h>
#include <boot/x86_desc.h>
#include <pro/pid.h>
#include <lib.h>
#include <drivers/fs.h>
#include <access.h>
#include <errno.h>  

/* This map will indicate which task is active now */
process_t *sched;   /* Process 0 (swapper process or the scheduler always hlt this time)*/
process_t *init;    /* Process 1 (init process) */
process_union *task_map[TASK_COUNT];    /* Currently process 2 (the shell) and 3 */
pid_t pid;

extern void do_halt(uint32_t ebp, uint32_t esp, uint8_t status);

static int32_t process_create(void);
static void process_free(pid_t _pid);
static int32_t context_switch(process_t *p);
static int32_t parse_arg(int8_t* command, int8_t *fname);
static void update_tss(pid_t _pid);


/**
 * @brief create a kernel process 0
 * 
 */
void idle(void) {
    asm volatile (".1: hlt; jmp .1;");
}


/**
 * @brief create a user process wit pid 2 (the shell)
 * 
 * This process will be created by fork in the future
 */
void shell_init(void) {
    pidmap_init();
    sys_execute("shell");
}


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
    if (pid == 2) 
        context_switch(CURRENT);

    /* If this is not the shell */
    process_t *curr = CURRENT;
    process_t *parent = CURRENT->parent;
    parent->child = NULL;
    update_tss(parent->pid);
    process_free(curr->pid);

    do_halt(parent->ebp_, parent->esp_, status);

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
    int32_t errno;
    int8_t fname[NAMESIZE + 1];
    uint32_t EIP_reg;
    memset(fname, 0, NAMESIZE + 1);
    /* parse arguments */
    if ((errno = parse_arg((int8_t *)cmd, fname)) < 0) 
        return errno;

    /* create process */
    if ((errno = process_create()) < 0)
        return errno;

    /* executable check and load program image into user's memory */
    if ((errno = pro_loader(fname, &EIP_reg)) < 0) {
        process_free(pid);
        pid = kill_pid();
        return errno;
        
    }

    /* init file array */
    fd_init();

    CURRENT->eip = EIP_reg;
    CURRENT->esp = USER_STACK_ADDR;

    asm("movl %%esp, %0" : "=r"(CURRENT->esp_));
    asm("movl %%ebp, %0" : "=r"(CURRENT->ebp_));

    /* switch to user mode (ring 3) */
    context_switch(CURRENT);

    return 0;
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
        return -EINVAL;
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
    if ((pid = alloc_pid()) < 0) 
        return pid;

    task_map[pid-2] = (process_union *)alloc_kstack(pid-2);
    
    /* setup current pid */
    CURRENT->pid = pid;

    /* setup the kernel stack info */

    /* set the parent pointer */
    if (pid == 2) {
        CURRENT->parent = NULL;
    } else {
        CURRENT->parent = &task_map[0]->process;
        task_map[0]->process.child = CURRENT;
    }

    return 0;
}


/**
 * @brief Free a existing process_t
 * 
 */
static void process_free(pid_t _pid) {
    task_map[_pid-2] = NULL;
    kill_pid();
    // user_mem_unmap(_pid);
}


/**
 * @brief Perform a context switch to ring 3 (user mode)
 * 
 * @return int32_t : 0 
 */
static int32_t context_switch(process_t *p) {
    /* (bottom) SS(handle by iret), DS, ESP, EFLAGS, CS, EIP (top) */
    /* popl and or are for getting esp, and set IF in EFLAGS
     * so that intrrupts will be reenabled in user mode. */
    
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
                  : "a"(USER_DS), "b"(p->esp), "c"(USER_CS), "d"(p->eip)
                  : "memory"
    );      
    return 0;
}

static void update_tss(pid_t _pid) {
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KERNEL_STACK_BEGIN - (_pid - 1) * KERNEL_STACK_SZ - 0x4;
}

