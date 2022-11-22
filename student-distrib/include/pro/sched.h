#ifndef _SCHED_H_
#define _SCHED_H_

#include <types.h>
#include <list.h>
 

/* a runqueue use for scheduling */
typedef struct {
    uint32_t n_active;      /* number of process linked into the lists */
    uint32_t bitmap[5];     /* a priority bit map that each flag is set 
                             * if and only if the corresponding priority 
                             * list is not empty */
    list_head staircase[140];   /* 140 lists for each priority */
} prio_array_t;


void sched_init(void);
int32_t schedule(void);
void pause(void);


#endif /* _SCHED_H_ */
