#ifndef _SCHED_H_
#define _SCHED_H_

#include <types.h>
#include <list.h>
#include <rbtree.h>

/* sets a target for is approximation of the "infinitely small" 
 * scheduling duration in perfect multitasking */
#define TARGET_LATENCY      48000000ULL     /* 48 ms */

/* a small granularity time used for sleeping process to avoid too many context switches */
#define WAKEUP_GRANULARITY  1000000ULL

#define NR_LATENCY          (TARGET_LATENCY / WAKEUP_GRANULARITY)   /* 8 */

#define NICE_0_LOAD         1024            /* the weight for process has nice value 0 */

/* minimum guanularity running time for each task */
#define MIN_GRANULARITY     6000000ULL      /* 6 ms */

/* lowest prio and weights for process 0 */
#define WEIGHT_IDLE         3               /* weight for process 0 */
#define WMULT_IDLE          1431655765      /* the inv_weight for process 0 */

#define WMULT_SHIFT         32               

/* walk up scheduling entities hierarchy (NOT USED IN THIS VERSION) */
#define for_each_sched(se) \
		for (; se; se = se->parent)


/* get sched info from a rb_node */
#define	sched_of(ptr) rb_entry(ptr, sched_t, node)


/* weight info */
typedef struct {
    uint32_t weight;
    uint32_t inv_weight;
} weight_t;


/* sched process info */
typedef struct {
    rb_node  node;              /* a red-block tree node for this thread */
    list_head wait_node;        /* use linked list of nodes to store waiting tasks*/
    weight_t load;              /* calculated load weight of each particle combined into one entity */
    uint64_t vruntime;          /* for how long this entity has run on a CPU, based on a virtual clock */
    uint64_t exec_start;        /* time when the process start running */
    uint64_t sum_exec_time;     /* time the process has been running in total */
    uint64_t prev_sum_exec_time;/* used for storing the previous run time of a process */
    int8_t   on_rq;             /* does the process on runqueue now? */
} sched_t;


/* Completely Fair Scheduler info:
 * 
 * NOTE:
 * min_vruntime is the smallest vruntime on the runqueue. 
 * This variable maintains the time for a task which has run for 
 * the least amount of time on the CPU. Note, that this is a single 
 * variable for the entire runqueue, so it serves as a singular 
 * benchmark for all the tasks to be compared against. The task, 
 * which has run the least, can be found out by subtracting min_vruntime
 * from the task’s vruntime. If the resulting number is big, the task 
 * clearly has run for long – the smaller the result, the sooner the 
 * task will get the CPU again.
 * 
 * rb_leftmost is the element that will be scheduled next. In an rbtree 
 * it is the leftmost element, because the tree is sorted based on 
 * vruntime of processes (smaller vruntime means sooner execution).
 */
typedef struct {
    weight_t load;          /* sum of weights of all tasks in the queue */
    uint32_t nr_running;    /* number of runnable tasks in the queue */
    uint64_t min_vruntime;  /* current min vruntime in the queue */
    uint32_t clock;         /* time clock in nanosecond */
    rb_node *left_most;     /* current leftmost rb-tree node */
    rb_node root;           /* root of the red-black tree */
    sched_t *current;       /* current running task's sched info (NULL when no process is running) */
} cfs_rq;



extern cfs_rq *rq;
extern const uint32_t sched_prio_to_weight[40];
extern const uint32_t sched_prio_to_wmult[40];


void sched_init(void);
void sched_fork(thread_t *task);
void task_tick(sched_t *curr);
void schedule(void);
void pause(void);


#endif /* _SCHED_H_ */
