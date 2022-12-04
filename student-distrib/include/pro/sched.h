#ifndef _SCHED_H_
#define _SCHED_H_

#define TIMESLICE   50         /* 500 ms of time slice for each task */

#include <types.h>
#include <list.h>

typedef struct {
    list_head *run_queue;       /* running queue containning all runnable processes */
    uint32_t  size;             /* number of tasks present in the queue */
} rr_rq_t;

extern rr_rq_t *rr_rq;

void sched_init(void);
void sched_tick(void);
void schedule(void);
void pause(void);

#endif  /* _SCHED_H_ */
