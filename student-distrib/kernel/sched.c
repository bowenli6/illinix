#include <pro/sched.h>
#include <pro/process.h>
#include <access.h>
#include <lib.h>


#define LOWESTPRI   139
#define HIGHESTPRI  0
#define BITMAP_SIZE 32
#define QUANTUM(p)  ((p) < 120 ? ((140 - (p)) * 20) : ((140 - (p)) * 5))
#define REALTIME(p) ((p) < 100)
#define NORMAL(p)   (!(REALTIME(p)))
#define SET_BIT(number, n)      ((number) |= 1UL << (n))
#define CLEAR_BIT(number, n)    ((number) &= ~(1UL << (n)))
#define CHANGE_BIT(number, n)   ((number) ^= 1UL << (n))


prio_array_t *runqueue;

static void degrade(thread_t *t);
static int32_t real_time_sched(thread_t *t);
static int8_t get_max_bit(uint8_t normal);
static thread_t *get_task(uint8_t prio);


/**
 * @brief init the scheduler (process 0)
 * 
 */
void sched_init(void) {
    int i;

    /* allocate memory spaces for kernel threads */
    process_t *swapperp = (process_t *) alloc_kstack(0);
    process_t *initp = (process_t *) alloc_kstack(1);

    /* set up process 0 */
    sched = &swapperp->thread;
    sched->state = RUNNABLE;
    sched->parent = NULL;
    sched->child = init;
    sched->kthread = 1;
    sched->argc = 1;
    strcpy(sched->argv, "scheduler");
    
    /* set up process 1 */
    init = &initp->thread;
    init->state = RUNNABLE;  
    init->parent = sched;
    init->child = NULL;
    init->kthread = 1;   
    init->argc = 1;
    strcpy(sched->argv, "init");
    
    /* create task list */
    task_head = &(sched->task);
    task_head->next = task_head;
    task_head->prev = task_head;
    list_add(&(init->task), task_head);

    /* create prio list */
    runqueue = kmalloc(sizeof(prio_array_t));

    /* init each level of staircase to an empty head */
    for (i = 0; i < 140; ++i) {
        runqueue->staircase[i].next = &runqueue->staircase[i];
        runqueue->staircase[i].prev = &runqueue->staircase[i];
    }
    runqueue->n_active = 0;

    /* start to running the task of process 0 */
    swapper();
}


/**
 * @brief scheduler service routine
 * 
 * using the staircase scheduler algorithm
 * referene: https://lwn.net/Articles/87729/
 * 
 * @return int32_t : 0 only if there are no tasks can be scheduled
 */
int32_t schedule(void) {
    thread_t *t;
    int8_t prio;

    GETPRO(t);

    /* avoid deadlock by ensuring that devices can interrupt */
    sti();

    /* degrade the current process */
    degrade(t);

    /* check if the task is real time */
    if (REALTIME(t->prio)) return real_time_sched(t);
    
    /* get the current best prio task */
    if ((prio = get_max_bit(1)) >= 0) {
        context_switch(sched->context, get_task(prio)->context);
    }

    /* no task can be scheduled 
     * return, pause, and wait for 
     * awaking by a timer interrupt */
    return 0;
}



static void degrade(thread_t *t) {
    uint8_t level = t->level;

    /* edge case when the prio is LOWESTPRI and degrade the first time */
    if (t->level == t->prio) {

        if (t->level == LOWESTPRI) return;

    } else {

        /* it fall into the bottom */
        if (t->level == LOWESTPRI) {
            t->level = t->prio;
            t->time_slice += t->time_slice_base;
            t->epoch++;
        }

    }

    /* remove the task from the staircase */
    list_del(&t->task);

    /* clear bit when no more task in the level*/
    if (list_empty(task_head)) 
        CLEAR_BIT(runqueue->bitmap[level / BITMAP_SIZE], level % BITMAP_SIZE);

    /* add task to the staircase with level++ */
    list_add(&t->task, &runqueue->staircase[++t->level]);
    SET_BIT(runqueue->bitmap[t->level / BITMAP_SIZE], t->level % BITMAP_SIZE);
}



/**
 * @brief Get the task object
 * 
 * @param prio 
 * @return thread_t* 
 */
static thread_t *get_task(uint8_t prio) {
    return runqueue->staircase[prio].next;
}


/**
 * @brief 
 * 
 * @param t 
 * @return int32_t 
 */
static int32_t real_time_sched(thread_t *t) {
    /* NOT IMPLEMENTED */
    return 0;
}


/**
 * @brief Get the max bit object
 * 
 * @param normal 
 * @return int8_t 
 */
static int8_t get_max_bit(uint8_t normal) {
    int i;
    int prio;

    /* no active task */
    if (!runqueue->n_active) return -1;
    
    /* if normal: i = 3; i < 6 */
    /* if not normal i = 0; i < 3 */
    for (i = normal * 3; i < (6 - (!normal) * 3); ++i) {
        
        /* check leading zeros to find the prio */
        if ((prio =__builtin_clz(runqueue->bitmap[i])) < BITMAP_SIZE) {

            /* if normal with prio in [96, 99] */
            if (normal && i == 3 && prio < 4) continue;

            /* if not normal with prio in [100, 127] */
            if (!normal && i == 3 && prio >= 4) continue;

            return BITMAP_SIZE * i + prio;
        }
    }

    return -1;
}




/**
 * @brief halts the central processing unit (CPU) until 
 * the next external interrupt is fired.
 * 
 */
void pause(void) {
    /* avoid deadlock by ensuring that devices can interrupt */
    sti();

    /* Spin (nicely, so we don't chew up cycles) */
    asm volatile (".1: hlt; jmp .1;");
}
