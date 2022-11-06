#ifndef _PROCESS_H
#define _PROCESS_H

#include <types.h>
#include <vfs/ece391_vfs.h>


#define COMMAND_LEN             128
#define TASK_COUNT              2               /* For cp3 we only need to support two tasks */

/* kernel, physical addr */
#define KERNEL_STACK_BEGIN       0x800000        /* The pyphisical addr begin at the end of 8MB; it should go up wards*/
#define KERNEL_STACK_SZ          0x2000          /* Per kernel stack size is 8KB */
#define USR_STACK_SZ             0x400000       /* 4 MB */

/* user-level, physical addr */
#define  FIRST_USR_BEGIN        0x800000        /* The first user addr begin at 8MB (to 12 MB) */
#define  SECOND_USR_BEGIN       0xC00000        /* The first user addr begin at 12MB (to 16 MB) */


 /* these macros are refered to gdt table */
#define GDT_KERNEL_CS           16
#define GDT_KERNEL_DS           24
#define GDT_USR_CS              32
#define GDT_USR_DS              40  
#define USR_LEVEL               3
#define KERNEL_LEVEL            0


/* user-level, virtual addr */
#define VIR_MEM_BEGIN           0x08000000      /* The mem begins at 128MB */
#define PROGRAM_IMG_BEGIN       0x08048000      /* The program img begin */
#define EIP_OFFSET              24              /* The EIP you need to jump to is the entry point from bytes 24-27 of
                                                    the executable that you have just loaded*/

typedef uint32_t pid_t;
typedef uint32_t gid_t;

typedef struct process {
    pid_t                   pid;                    /* process id number */
    // uint8_t                 arg;                    /* store the arguments */
    struct process          *parent_addr;           /* parent process addr */

    uint32_t                tss_ESP0;               /* where the kenel mode stak/PCB begins */
    uint16_t                tss_SS0;    
    uint32_t                tss_ESP;
    uint16_t                tss_SS;

    uint32_t                usr_phy_addr;           /* pyhisical address of user mode stak; 8-12; 12-16 */
    uint32_t                kernel_phy_addr;        /* pyhisical address of PCB; 8-12; 12-16 */

    vfs_t                   pro_vfs;
    files                   pro_files;                  

    uint32_t                signal;
    //
} process_t;


/* Two 4 KB pages containing both the process descriptor and the kernel stack. */
typedef union {
    process_t process;
    uint32_t stack[2048];
} process_union;

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

extern process_t     task_map[TASK_COUNT];
extern process_t     *curr_process;




#endif /* _PROCESS_H */
