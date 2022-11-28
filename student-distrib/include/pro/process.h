#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <vfs/ece391_vfs.h>
#include <drivers/terminal.h>
#include <access.h>
#include <pro/sched.h>
#include <list.h>


#define ARGSIZE         33              /* max size of a command argument*/           
#define MAXARGS         10              /* max number of arguments */
#define SHELL           "shell"         /* shell program */
#define TASKSTART       2               /* user tasks starts from 2 */
#define NICE_INIT       10              /* nice value for init process */
#define NICE_SHELL      0               /* nice value for shell process */
#define NICE_NORMAL     5               /* nice value for default process */
#define NTERMINAL       3
#define MAXCHILDREN     100     

#define task_of(ptr)  container_of(ptr, thread_t, sched_info)

#define NEED_RESCHED    1               /* flag used for rescheduling */

typedef enum { UNUSED, RUNNING, RUNNABLE, SLEEPING, EXITED, ZOMIBIE } pro_state;



/* hardware context */
typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t eip;
    uint32_t eflags;
    uint16_t es;
    uint16_t cs;
    uint16_t ds;
    uint16_t fs;
    uint16_t gs;
} context_t;


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
    uint32_t           sbrk;            /* heap pointer */
    files              *fds;            /* opened file descritors */
    uint8_t            kthread;         /* 1 if this thread is belong to the kernel */
    terminal_t         *terminal;       /* terminal for this thread (shell only) */
    int8_t             nice;            /* nice value */
} thread_t;

/* array of terminals for each shells */
typedef struct {
    terminal_t **terminals;
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

void do_exit(uint32_t status);
void do_halt(uint32_t status);
int32_t do_fork(thread_t *parent, uint8_t kthread);
int32_t do_execute(const int8_t *cmd);
pid_t do_getpid(void);
void *do_sbrk(uint32_t size);

uint32_t get_esp0(pid_t pid);
void context_switch(thread_t *from, thread_t *to);

/* implemented in switch.S */
void save_context(context_t *context);
void swtch(context_t *from, context_t *to);


#endif /* _PROCESS_H_ */
