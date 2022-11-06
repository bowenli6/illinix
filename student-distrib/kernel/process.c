#include <pro/process.h>
#include <boot/syscall.h>
#include <boot/x86_desc.h>
#include <drivers/fs.h>
#include <access.h>
#include <errno.h>  

/* this map will indicate which task is active now */
process_union *task_map[TASK_COUNT];
int32_t curr_pid;
// tss is the current tss we are using


static int32_t parse_arg_to_process(uint8_t* command, uint8_t* stored_pro, uint8_t* stored_file);

void process_init() {
    curr_pid = -1;
}



asmlinkage int32_t sys_halt(uint8_t status) {
    return 0;
}

asmlinkage int32_t sys_execute(const uint8_t *cmd) {
    int32_t errno, EIP_reg;
    int8_t fname[NAMESIZE];


    if ((errno = parse_arg_to_process(cmd, NULL, fname)) < 0) 
        return errno;

    /* open the program file */
    if ((EIP_reg = pro_loader(fname)) < 0) 
        return EIP_reg;   

    /* check file */
    if ((errno = process_create()) < 0)
        return errno;
    
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
 * @brief store the entire command line witout the leading file name
 * 
 * @return 0  if the program executes a halt system call
 * @return -1 if the command cannot be executed
 */
static int32_t parse_arg_to_process(uint8_t* command, uint8_t* stored_pro, uint8_t* stored_file) {
    uint8_t i = 0, j = 0;

    if (command == NULL) {
        printf("The command itself is null, we can get no information. \n");
        return -1;                          /* return -1 if the command can not be executed */
    }

    /* parse the command into file name and the rest of the command */
    
    while (command[i] != '\0') {

        if (command[i] == ' ') {
            stored_file[i] = '\0';
            i += 1;
            break;
        }

        stored_file[i] = command[i];
        i += 1;
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
 * @brief this function is used user mode to kernel mode
 * 
 * @return none
 */
void usr_to_kernel() {
    return;
}


/**
 * @brief this function is used for kernel mode to usr mode
 * 
 * @return none
 */
void kernel_to_usr(int32_t EIP_reg) {
    
    uint32_t EFLAGS_reg, ESP_reg;
    uint16_t CS_reg, DS_reg, SS_reg;

    // EIP_reg
    CS_reg = GDT_USR_CS | USR_LEVEL;
    DS_reg = GDT_USR_DS | USR_LEVEL;
    EFLAGS_reg = tss.eflags;
    ESP_reg = USR_STACK_SZ + VIR_MEM_BEGIN;                 /* which starts from 132MB */

    /* store the ss0 and esp0 to the tss */
    tss.ss0 = current()->tss_SS0;
    tss.esp0 = current()->tss_ESP0;


    // asm volatile("              \n\
            
    //         "
    //         : "=a"(val)
    //         : "d"(port)
    //         : "memory"
    // );

}


static int32_t process_create() {
    int32_t errno;

    if (++curr_pid < 0) {
        return -EINVAL;
    }
    if (curr_pid >= 2) {    /* Reach the maxium value of processes */
        return -EINVAL;
    }

    task_map[curr_pid] = (process_union *)alloc_kstack(curr_pid);
    
    /* setup current pid */
    task_map[curr_pid]->process.pid = curr_pid;

    /* setup the kernel stack info */
    task_map[curr_pid]->process.tss_ESP0 = (uint32_t) task_map[curr_pid] + KERNEL_STACK_SZ;
    task_map[curr_pid]->process.tss_SS0 = task_map[curr_pid]->process.tss_ESP0;

    /* set the parent pointer */
    if (curr_pid == 0) {
        task_map[curr_pid]->process.parent_addr = NULL;
    } else {
        task_map[curr_pid]->process.parent_addr = (process_t*)(&task_map[0]->process);
    }

    /* setup user level stack info */

    return curr_pid;
}


process_t *current() {
    return &task_map[curr_pid]->process;
}
