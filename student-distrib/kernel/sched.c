#include <pro/sched.h>
#include <pro/process.h>
#include <access.h>
#include <lib.h>


#define LOWESTPRI   139
#define HIGHESTPRI  0
#define MIN_TIMESLICE 10    /* 10 ms */
#define BITMAP_SIZE 32
#define QUANTUM(p)  ((p) < 120 ? ((140 - (p)) * 20) : ((140 - (p)) * 5))
#define REALTIME(p) ((p) < 100)
#define NORMAL(p)   (!(REALTIME(p)))
#define SET_BIT(number, n)      ((number) |= 1UL << (n))
#define CLEAR_BIT(number, n)    ((number) &= ~(1UL << (n)))
#define CHANGE_BIT(number, n)   ((number) ^= 1UL << (n))




/*
 * Nice levels are multiplicative, with a gentle 10% change for every
 * nice level changed. I.e. when a CPU-bound task goes from nice 0 to
 * nice 1, it will get ~10% less CPU time than another CPU-bound task
 * that remained on nice 0.
 *
 * The "10% effect" is relative and cumulative: from _any_ nice level,
 * if you go up 1 level, it's -10% CPU usage, if you go down 1 level
 * it's +10% CPU usage. (to achieve that we use a multiplier of 1.25.
 * If a task goes up by ~10% and another task goes down by ~10% then
 * the relative distance between them is ~25%.)
 */
const uint32_t sched_prio_to_weight[40] = {
    /* -20 */     88761,     71755,     56483,     46273,     36291,
    /* -15 */     29154,     23254,     18705,     14949,     11916,
    /* -10 */      9548,      7620,      6100,      4904,      3906,
    /*  -5 */      3121,      2501,      1991,      1586,      1277,
    /*   0 */      1024,       820,       655,       526,       423,
    /*   5 */       335,       272,       215,       172,       137,
    /*  10 */       110,        87,        70,        56,        45,
    /*  15 */        36,        29,        23,        18,        15,
};


/*
 * Inverse (2^32/x) values of the sched_prio_to_weight[] array, precalculated.
 * 
 * sched_prio_to_wmult[i] = 2^32 / sched_prio_to_weight[i]
 * 
 * In cases where the weight does not change often, we can use the
 * precalculated inverse to speed up arithmetics by turning divisions
 * into multiplications:
 */
const uint32_t sched_prio_to_wmult[40] = {
    /* -20 */     48388,     59856,     76040,     92818,    118348,
    /* -15 */    147320,    184698,    229616,    287308,    360437,
    /* -10 */    449829,    563644,    704093,    875809,   1099582,
    /*  -5 */   1376151,   1717300,   2157191,   2708050,   3363326,
    /*   0 */   4194304,   5237765,   6557202,   8165337,  10153587,
    /*   5 */  12820798,  15790321,  19976592,  24970740,  31350126,
    /*  10 */  39045157,  49367440,  61356676,  76695844,  95443717,
    /*  15 */ 119304647, 148102320, 186737708, 238609294, 286331153,
};


/* local helper functions */

static int8_t nice_to_index(int8_t nice);
static thread_t *pick_next_task(thread_t *curr);
static thread_t* real_time_sched(thread_t *curr);



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


    /* start to running the task of process 0 */
    swapper();
}


/**
 * @brief scheduler service routine
 * 
 * using the Linux Completely Fair scheduler (CFS) algorithm
 * 
 * referene: 
 * Love, Robert, Linux Kernel Development, Chapter 4
 * https://www.doc-developpement-durable.org/file/Projets-informatiques/cours-&-manuels-informatiques/Linux/Linux%20Kernel%20Development,%203rd%20Edition.pdf
 * https://lwn.net/Articles/87729/
 * https://www2.hawaii.edu/~esb/2004fall.ics612/dec06.html
 * https://static.linaro.org/connect/yvr18/presentations/yvr18-220.pdf
 * 
 * 
 * @return int32_t : 0 only if there are no tasks can be scheduled
 */
void schedule(void) {
    thread_t *curr, *next;
    int8_t prio;

    GETPRO(curr);

    /* avoid deadlock by ensuring that devices can interrupt */
    sti();

    /* find the next task to run */
    next = pick_next_task(curr);

    context_switch(curr->context, next->context);
}



/**
 * @brief nice[-20, 19] => index[0, 39]
 * 
 * @param nice : nice value 
 * @return int8_t : index to the sched_prio_to_x array
 */
static int8_t nice_to_index(int8_t nice) {
    return nice + 20;
}



/**
 * @brief pick the next task to run
 * 
 * @param curr : the current running thread 
 * @return thread_t* : the pointer to next runable thread
 */
static thread_t *pick_next_task(thread_t *curr) {
    return NULL;
}



/**
 * @brief schedule for a real-time process
 * 
 * @param curr : the current running thread 
 * @return thread_t* : the pointer to next runable thread
 */
static thread_t* real_time_sched(thread_t *curr) {
    /* NOT IMPLEMENTED */
    return 0;
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
