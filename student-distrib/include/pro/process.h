#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <vfs/vfs.h>
#include <drivers/terminal.h>
#include <access.h>
#include <pro/cfs.h>
#include <list.h>


#define ARGSIZE         33              /* max size of a command argument*/           
#define MAXARGS         10              /* max number of arguments */
#define SHELL           "shell"         /* shell program */
#define INIT            "init"          /* init program */
#define IDLE            "idle"          /* idle program */
#define TASKSTART       2               /* user tasks starts from 2 */
#define NICE_INIT       19              /* nice value for init process */
#define NICE_SHELL      5              /* nice value for shell process */
#define NICE_NORMAL     0               /* nice value for default process */
#define NTERMINAL       3               /* max number of terminals supported */
#define MAXCHILDREN     100             /* default max number of children for a process */
#define STACK           2042            /* CPU pushs user registers on stack starting at this offset */
#define USEREIP         2042            /* CPU pre-pushed user eip needed to be copied */
#define USERESP         2045            /* CPU pre-pushed user esp needed to be copied */
#define KSTACK_SIZE     2048            /* 2048 word */

#define task_of(ptr)  container_of(ptr, thread_t, sched_info)

#define NEED_RESCHED    1               /* flag used for rescheduling */
#define WAKEUP          2               /* flag used for waking up */

typedef enum { UNUSED, RUNNING, RUNNABLE, SLEEPING, EXITED, ZOMIBIE } pro_state;


/* deprecated */
#define __swtch(prev, next)                                 \
do {                                                  \
    asm volatile (" pushfl                          \n\t"   /* push eflags */               \
                  " pushl %%ebp                     \n\t"   /* push ebp */                  \
                  " movl  %%esp, %[prev_esp]        \n\t"   /* save esp to prev */          \
                  " movl  %[next_esp], %%esp        \n\t"   /* restore esp from next */     \
                  " movl  $1f, %[prev_eip]          \n\t"   /* save eip to prev */          \
                  " pushl %[next_eip]               \n\t"   /* push eip from next */        \
                  " ret                             \n\t"   /* jump to swtch (switch.S) */  \
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
    );                                                           \
} while (0)                                                  \


/* hardware context (callee saved registers and part of segment registers) */
typedef struct {
    uint32_t eip;
    uint32_t esp;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t eax;
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
    list_head          wait_node;       /* a list of all sleeping tasks */
    list_head          run_node;        /* a list of all runnable tasks */
    volatile uint32_t  count;           /* time slice for a task */
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
    uint32_t           console_id;      /* console for this thread */
    int32_t            nice;            /* nice value */
    uint8_t            **user_vidmap;
} thread_t;


/* array of terminals for each shells */
typedef struct {
    uint32_t id;
    uint32_t fkey;
    thread_t *task;
    uint8_t* vidmap;
    uint32_t intr_flag;       /* interrupt CLI/STI flag */
} console_t;


/* two 4 KB pages containing both the process descriptor and the kernel stack. */
typedef union {
    thread_t thread;
    uint32_t stack[KSTACK_SIZE];
} process_t;


extern thread_t *idle;
extern thread_t *init;
extern list_head task_queue;  
extern list_head wait_queue;
extern console_t **consoles;
extern console_t *current;

void swapper(void);
void init_task(void);
void inline context_switch(thread_t *prev, thread_t *next);
void process_free(thread_t *current);

void do_exit(uint32_t status);
int32_t do_execv(thread_t *curr, const int8_t *pathname, int8_t *const argv[]);
int32_t do_fork(thread_t *parent, uint8_t kthread);
int32_t do_execute(thread_t *parent, const int8_t *cmd);
pid_t do_getpid(void);
void *do_sbrk(uint32_t size);

uint32_t get_esp0(thread_t *curr);
int32_t file_init(int32_t fd, file_t *file, dentry_t *dentry, file_op *op, thread_t *curr);
thread_t **children_create(void);

/* implemented in fs.c */

int32_t pro_loader(const int8_t *fname, uint32_t *EIP, thread_t* curr);

/* implemented in switch.S */
void swtch(context_t *prev, context_t *next);

/* implemented in access.c */

void __umap(thread_t *from, thread_t *to);
void create_vm(thread_t* t);
void user_mem_map(thread_t* t);
void user_mem_unmap(thread_t* t);
void free_vm(thread_t* t);

/* implemented in vfs.c */

int32_t fd_init(thread_t *curr);
int32_t __open(int32_t fd, const int8_t *fname, file_type_t type, file_op *op, thread_t *curr);

/* implemented in file.c */

int32_t file_init(int32_t fd, file_t *file, dentry_t *dentry, file_op *op, thread_t *curr);

/* implemented in sched.c */

void __schedule(thread_t *curr);
void sched_fork(thread_t *task);
void sched_sleep(thread_t *task);
void sched_wakeup(thread_t *from, thread_t *task);
void enqueue_task(thread_t *new, int8_t wakeup);
void sched_exit(thread_t *child, thread_t *parent);
void activate_task(thread_t *task);
void wakeup_preempt(thread_t *task);
void task_tick(thread_t *curr);

#endif /* _PROCESS_H_ */
