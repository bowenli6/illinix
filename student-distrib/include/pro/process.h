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
#define NICE_INIT       5               /* nice value for init process */
#define NICE_SHELL      5               /* nice value for shell process */
#define NICE_NORMAL     0               /* nice value for default process */

#define task_of(ptr)  container_of(ptr, thread_t, sched_info)


typedef enum { RUNNING, RUNNABLE, SLEEPING, STOPPED, EXITED } pro_state;


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
    int32_t            argc;            /* number of arguments */
    int8_t             **argv;          /* user command line argument */
    pid_t              pid;             /* process id number */
    struct thread      *parent;         /* parent process addr */
    struct thread      *child;          /* child process addr */
    context_t          *context;        /* hardware context */
    uint32_t           usreip;          /* user eip */
    uint32_t           usresp;          /* user esp */
    files              *fds;            /* opened file descritors */
    uint8_t            kthread;         /* 1 if this thread is belong to the kernel */
    terminal_t         *terminal;       /* terminal for this thread (shell only) */
    int8_t             nice;            /* nice value */
} thread_t;


/* array of terminals for each shells */
typedef struct {
    terminal_t **terminals;
    uint32_t max;
    uint32_t size;
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
int32_t do_execute(const int8_t *cmd);
pid_t do_getpid(void);

uint32_t get_esp0(pid_t pid);
void save_context(context_t *context);
void context_switch(context_t *from, context_t *to);

#endif /* _PROCESS_H_ */
