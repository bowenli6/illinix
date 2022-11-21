#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <types.h>
#include <vfs/ece391_vfs.h>
#include <drivers/terminal.h>


#define COMMAND_LEN     33
#define MAXARGS         10
#define SHELL           "shell"
#define NTASK           32              /* max number of user tasks */
#define TASKSTART       2               /* user tasks starts from 2 */
  
          

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

typedef enum {
    UNRUNNABLE,
    RUNNABLE,
    RUNNING,
    STOPPED,
    SLEEPING
} pro_state;

typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t eflags;
    uint16_t es;
    uint16_t cs;
    uint16_t ds;
    uint16_t fs;
    uint16_t gs;
} context_t;

typedef struct thread {
    volatile pro_state state;	        /* process state */
    int32_t            argc;            /* number of arguments */
    int8_t             argv[MAXARGS][COMMAND_LEN]; /* user command line argument */
    pid_t              pid;             /* process id number */
    gid_t              gid;             /* process group id*/
    struct thread     *parent;          /* parent process addr */
    struct thread     *child;           /* child process addr */
    context_t         *context;         /* hardware context */
    uint32_t           eip;
    files              fds;             /* opened file descritors */
    uint8_t            kthread;         /* 1 if this thread is belong to the kernel */
} thread_t;


typedef struct {
    terminal_t **terminals;
    uint32_t max;
    uint32_t size;
} console_t;


/* Two 4 KB pages containing both the process descriptor and the kernel stack. */
typedef union {
    thread_t thread;
    uint32_t stack[2048];
} process_t;


extern pid_t curr_pid;
extern thread_t *tasks[NTASK];
extern thread_t *init;
extern console_t *console;

#define GETPRO(pid) (tasks[(pid)-TASKSTART])

#define CURRENT GETPRO(curr_pid)

void swapper(void);
void init_task(void);
void do_halt(uint32_t status);
int32_t do_execute(const int8_t *cmd);
void switch_to_user(thread_t *p);
uint32_t get_esp0(pid_t pid);
void save_context(context_t *context);
void context_switch(context_t *from, context_t *to);

#endif /* _PROCESS_H_ */
