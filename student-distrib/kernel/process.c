#include <pro/process.h>
#include <boot/syscall.h>
#include <boot/x86_desc.h>
#include <drivers/fs.h>
#include <access.h>
#include <errno.h>  

/* This map will indicate which task is active now */
process_t *sched;   /* Process 0 (swapper process or the scheduler always hlt this time)*/
process_t *init;    /* Process 1 (init process) */
process_union *task_map[TASK_COUNT];    /* Currently process 2 (the shell) and 3 */
pid_t pid;


static int32_t process_create(void);
static int32_t context_switch(void);
static int32_t parse_arg(uint8_t* command, uint8_t *args, uint8_t *fname);


/**
 * @brief create a kernel process 0
 * 
 */
void idle(void) {
    asm ("            \n\
        0: hlt        \n\
        jmp 0b        \n\
        "   
    );
}


/**
 * @brief create a user process wit pid 2 (the shell)
 * 
 * This process will be created by fork in the future
 */
void shell_init(void) {
    sys_execute((uint8_t)"shell");
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
    return 0;
}


/**
 * @brief A system call service routine for creating a process
 * The calling convation of this function is to use the 
 * arguments from the stack
 * 
 * @param cmd : A string contains the command of the process
 * @return int32_t : positive or 0 denote success, negative values denote an error condition
 */
asmlinkage int32_t sys_execute(const uint8_t *cmd) {
    int32_t errno, EIP_reg;
    int8_t fname[NAMESIZE];

    /* parse arguments */
    if ((errno = parse_arg(cmd, NULL, fname)) < 0) 
        return errno;

    /* executable check and load program image into user's memory */
    if ((EIP_reg = pro_loader(fname)) < 0) 
        return EIP_reg;   

    /* create process */
    if ((errno = process_create()) < 0)
        return errno;
    
    /* switch to user mode (ring 3) */
    context_switch();
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
static int32_t parse_arg(uint8_t *cmd, uint8_t *args, uint8_t *fname) {
    uint8_t i = 0, j = 0;

    if (cmd == NULL) {
        return -EINVAL;
    }

    /* parse the command into file name and the rest of the command */
    
    while (cmd[i] != '\0') {

        if (cmd[i] == ' ') {
            fname[i] = '\0';
            i++;
            break;
        }

        fname[i] = cmd[i];
        i++;
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
    int32_t errno;

    if ((errno = alloc_pid()) < 0) 
        return errno;

    task_map[pid-2] = (process_union *)alloc_kstack(pid-2);
    
    /* setup current pid */
    task_map[pid-2]->process.pid = pid;

    /* setup the kernel stack info */

    /* set the parent pointer */
    if (pid == 2) {
        task_map[pid-2]->process.parent = NULL;
    } else {
        task_map[pid-2]->process.parent = &task_map[pid-3]->process;
        task_map[pid-3]->process.child = &task_map[pid-2]->process;
    }

    return 0;
}


/**
 * @brief Perform a context switch to ring 3 (user mode)
 * 
 * @return int32_t : 0 
 */
static int32_t context_switch(void) {
    process_union *p = task_map[pid];
    /* (bottom) SS(handle by iret), DS, ESP, EFLAGS, CS, EIP (top) */
    /* popl and or are for getting esp, and set IF in EFLAGS
     * so that intrrupts will be reenabled in user mode. */

    asm volatile ("                         \n\
                    cli                     \n\
                    andl  $0xFF, %%eax      \n\
                    movw  %%ax, %%ds        \n\
                    movw  %%ax, %%es        \n\
                    movw  %%ax, %%fs        \n\
                    movw  %%ax, %%gs        \n\
                    pushl %%eax             \n\
                    pushl %%ebx             \n\
                    pushfl                  \n\
                    popl  %%eax             \n\
                    orl   $0x200, %%eax     \n\
                    pushl %%eax             \n\
                    pushl %%ecx             \n\
                    pushl %%edx             \n\
                    iret                    \n\
                  "
                  :
                  : "a"(USER_DS), "b"(p->process.esp), "c"(USER_CS), "d"(p->process.eip)
                  : "memory"
    );      
    return 0;
}

process_t *current(void) {
    return &task_map[pid]->process;
}
