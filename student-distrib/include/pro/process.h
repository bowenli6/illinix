#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <types.h>
#include <vfs/ece391_vfs.h>


#define COMMAND_LEN             128
#define MAXARGS                 10
#define TASK_COUNT              8               

/* kernel, physical addr */
#define KERNEL_STACK_BEGIN       0x800000        /* The pyphisical addr begin at the end of 8MB; it should go up wards*/
#define KERNEL_STACK_SZ          0x2000          /* Per kernel stack size is 8KB */
#define USR_STACK_SZ             0x400000       /* 4 MB */

/* user-level, physical addr */
#define  FIRST_USR_BEGIN        0x800000        /* The first user addr begin at 8MB (to 12 MB) */
#define  SECOND_USR_BEGIN       0xC00000        /* The first user addr begin at 12MB (to 16 MB) */


/* user-level, virtual addr */
#define VIR_MEM_BEGIN           0x08000000      /* The mem begins at 128MB */
#define USER_STACK_ADDR         (0x8400000 - 0x4)
#define PROGRAM_IMG_BEGIN       0x08048000      /* The program img begin */

typedef struct process {
    volatile long      state;	        /* -1 unrunnable, 0 runnable, >0 stopped */
    int32_t            argc;            /* number of arguments */
    pid_t              pid;             /* process id number */
    gid_t              gid;             /* process group id*/
    struct process     *parent;         /* parent process addr */
    struct process     *child;          /* child process addr */
    uint32_t           esp;
    uint32_t           ebp;
    uint32_t           eip;
    files              fds;                  
} process_t;


/* Two 4 KB pages containing both the process descriptor and the kernel stack. */
typedef union {
    process_t process;
    uint32_t stack[2048];
} process_union;

extern pid_t curr_pid;
extern process_union *task_map[TASK_COUNT];

#define CURRENT (&task_map[curr_pid-2]->process)
#define GETPRO(pid) (&task_map[(pid)-2]->process)

void swapper();
void init_task();

#endif /* _PROCESS_H_ */
