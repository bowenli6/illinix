#ifndef _SCHED_H_
#define _SCHED_H_

#include <types.h>
#include <list.h>
#include <pro/rbtree.h>

/* sets a target for is approximation of the "infinitely small" 
 * scheduling duration in perfect multitasking */
#define TARGET_LATENCT      48000000        /* 48 ms */

#define NICE_0_LOAD         1024            /* the weight for process has nice value 0 */

/* minimum guanularity running time for each task */
#define MIN_GRANULARITY     6000000         /* 6 ms */

/* lowest prio and weights for process 0 */
#define WEIGHT_SWAPPER      3               /* weight for process 0 */
#define WMULT_SWAPPER       1431655765      /* the inv_weight for process 0 */


/* map from nice value to weight info */
extern const uint32_t sched_prio_to_weight[40];
extern const uint32_t sched_prio_to_wmult[40];


/* weight info */
typedef struct {
    uint32_t weight;
    uint32_t inv_weight;
} weight_t;


/* sched process info */
typedef struct {
    rb_tree  node;          /* a red-block tree node for this thread */
    weight_t load;          /* the weight info of the process */
    uint32_t vruntime;      /* virtual runtime */
    uint32_t exec_time;     /* time when exec the process (nanosecond)*/
    uint32_t timeslice;     /* "time slice" for the process */
    int8_t   on_rq;         /* does the process on runqueue? */
    int8_t   nice;          /* nice value */
} sched_t;


/* cfs scheduler info */
typedef struct {
    uint32_t nrunning;      /* number of tasks in the queue */
    uint32_t min_vruntime;  /* current min vruntime in the queue */
    uint32_t clock;         /* time clock in nanosecond */
    uint32_t total_weights; /* total weights among all tasks in the queue */
    rb_tree *left_most;     /* current leftmost rb-tree node */
    sched_t *current;       /* current task's sched info */
} cfs_rq;


extern cfs_rq *runqueue;


void sched_init(void);
int32_t update_curr(sched_t *curr);
void set_sched_task(sched_t *new);
void schedule(void);
void pause(void);


#endif /* _SCHED_H_ */
