#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <vfs/vfs.h>
#include <drivers/terminal.h>
#include <access.h>
#include <pro/sched.h>
#include <list.h>


#define ARGSIZE         33              /* max size of a command argument*/           
#define MAXARGS         10              /* max number of arguments */
#define SHELL           "shell"         /* shell program */
#define INIT            "init"          /* init program */
#define TASKSTART       2               /* user tasks starts from 2 */
#define NICE_INIT       10              /* nice value for init process */
#define NICE_SHELL      0               /* nice value for shell process */
#define NICE_NORMAL     5               /* nice value for default process */
#define NTERMINAL       3               /* max number of terminals supported */
#define MAXCHILDREN     100             /* default max number of children for a process */
#define STACK           2043            /* CPU pushs user registers on stack starting at this offset */
#define NCONTEXT        5               /* 5 CPU pre-pushed user info needed to be copied */

#define task_of(ptr)  container_of(ptr, thread_t, sched_info)

#define NEED_RESCHED    1               /* flag used for rescheduling */

typedef enum { UNUSED, RUNNING, RUNNABLE, SLEEPING, EXITED, ZOMIBIE } pro_state;


#define swtch(prev, next)                             \
do {                                                  \
    asm volatile (" pushfl                          \n\t"   /* push eflags */               \
                  " pushl %%ebp                     \n\t"   /* push ebp */                  \
                  " movl  %%esp, %[prev_esp]        \n\t"   /* save esp to prev */          \
                  " movl  %[next_esp], %%esp        \n\t"   /* restore esp from next */     \
                  " movl  $1f, %[prev_eip]          \n\t"   /* save eip to prev */          \
                  " pushl %[next_eip]               \n\t"   /* push eip from next */        \
                  " pushl %[next_con]               \n\t"   /* push next's context */       \
                  " pushl %[prev_con]               \n\t"   /* push prev's context */       \
                  " jmp   __swtch                   \n\t"   /* jump to swtch (switch.S) */  \
                  " 1:                              \n\t"   /* eip address for prev */      \
                  " popl  %%ebp                     \n\t"   /* restore ebp */               \
                  " popfl                           \n\t"   /* restore eflags */            \
                                                             \
                  /* output */                               \
                  : [prev_esp] "=m"(prev->context->esp),     \
                    [prev_eip] "=m"(prev->context->eip)      \
                                                             \
                  /* input */                                \
                  : [next_esp] "m"(next->context->esp),      \
                    [next_eip] "m"(next->context->eip),      \
                    [prev_con] "m"(prev->context),           \
                    [next_con] "m"(next->context)            \
                                                             \
                  : "memory"                                 \
    );                                                       \
} while (0)                                                  \



/* hardware context (callee saved registers and part of segment registers) */
typedef struct {
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebp;
    uint32_t esp;
    uint32_t eip;
    uint32_t eflags;
    uint16_t es;
    uint16_t fs;
    uint16_t gs;
} context_t;

typedef struct vm_area {
    uint32_t*           mmap;      
    uint32_t            vmstart;
    uint32_t            vmend;
    uint32_t            vmflag;
    struct vm_area      *next;
} vm_area_t;

typedef struct vmem {
    struct vm_area      *map_list;
    uint32_t            size;
    uint32_t            file_length;
    uint32_t            start_brk;
    uint32_t            brk;
    int                 count;
} vmem_t;

/* define a thread that run as a process */
typedef struct thread {
    list_head          task_node;       /* a list of all tasks  */
    sched_t            sched_info;      /* info used for scheduler */
    volatile pro_state state;	        /* process state */
    volatile uint8_t   flag;            /* process flag */
    int32_t            argc;            /* number of arguments */
    int8_t             **argv;          /* user command line argument */
    pid_t              pid;             /* process id number */
    struct thread      *parent;         /* parent process addr */
    struct thread      **children;      /* child process addr */
    uint64_t           n_children;      /* number of children */
    uint64_t           max_children;    /* max number of children */
    context_t          *context;        /* hardware context */
    uint32_t           usreip;          /* user eip */
    uint32_t           usresp;          /* user esp */
    vmem_t             vm;              /* user virtual memory info */
    files              *fds;            /* opened file descritors */
    uint8_t            kthread;         /* 1 if this thread is belong to the kernel */
    terminal_t         *terminal;       /* terminal for this thread */
    int32_t            nice;            /* nice value */
} thread_t;


/* array of terminals for each shells */
typedef struct {
    terminal_t **terminals;
    int32_t    size;
} console_t;


/* two 4 KB pages containing both the process descriptor and the kernel stack. */
typedef union {
    thread_t thread;
    uint32_t stack[2048];
} process_t;


extern thread_t *idle;
extern thread_t *init;
extern list_head *task_queue;  
extern list_head *wait_queue;

void swapper(void);
void init_task(void);
void inline context_switch(thread_t *prev, thread_t *next);

void do_exit(uint32_t status);
void do_halt(uint32_t status);
int32_t do_fork(thread_t *parent, uint8_t kthread);
int32_t do_execute(const int8_t *cmd);
pid_t do_getpid(void);
void *do_sbrk(uint32_t size);

uint32_t get_esp0(thread_t *curr);
int32_t file_init(int32_t fd, file_t *file, dentry_t *dentry, file_op *op, thread_t *curr);
thread_t **children_create(void);

/* implemented in fs.c */

int32_t pro_loader(int8_t *fname, uint32_t *EIP, thread_t *curr);

/* implemented in switch.S */

void save_context(context_t *context);
void __swtch(context_t *from, context_t *to);

/* implemented in access.c */

void user_mem_map(thread_t* t);
void user_mem_unmap(thread_t* t);

/* implemented in vfs.c */

int32_t fd_init(thread_t *curr);
int32_t __open(int32_t fd, const int8_t *fname, file_type_t type, file_op *op, thread_t *curr);

/* implemented in file.c */

int32_t file_init(int32_t fd, file_t *file, dentry_t *dentry, file_op *op, thread_t *curr);

/* implemented in sched.c */

void sched_fork(thread_t *task);
void sched_sleep(thread_t *task);
void sched_wakeup(thread_t *task);

#endif /* _PROCESS_H_ */
