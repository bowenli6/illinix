#ifndef _SCHED_H_
#define _SCHED_H_

#include <types.h>
#include <list.h>
#include <pro/rbtree.h>

/* lowest prio and weights for process 0 */
#define WEIGHT_SWAPPER      3               /* weight for process 0 */
#define WMULT_SWAPPER       1431655765      /* the inv_weight for process 0 */


/* map from nice value to weight info */
extern const uint32_t sched_prio_to_weight[40];
extern const uint32_t sched_prio_to_wmult[40]


/* weight info */
typedef struct {
    uint32_t weight;
    uint32_t inv_weight;
} weight_t;


/* sched process info */
typedef struct {
    weight_t load;          /* the weight info of the process */
    rb_tree node;           /* a red-block tree node for this thread */
    uint32_t vruntime;      /* virtual runtime */
    uint32_t runtime;       /* actual runtime */
} sched_t;


/* cfs scheduler info */
typedef struct {
    rb_tree node;
    uint32_t min_vruntime;  
} cfs_rq;


void sched_init(void);
void schedule(void);
void pause(void);


#endif /* _SCHED_H_ */
