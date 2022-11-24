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


/* the running queue containing all runable threads */
cfs_rq *runqueue;


/* local helper functions */

static int8_t nice_to_index(int8_t nice);
static thread_t *pick_next_task(sched_t *curr);
static void enqueue_task(sched_t *curr, int8_t wakeup);
static inline thread_t *task_of(sched_t *s);
static inline sched_t *sched_of(rb_tree *node);
static void set_load_weight(thread_t *p);



/**
 * @brief init the scheduler (process 0)
 * 
 */
void sched_init(void) {
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

    /* create runqueue */
    runqueue = kmalloc(sizeof(cfs_rq));
    runqueue->clock = 0;
    runqueue->nrunning = 0;

    /* add init process to the run queue */
    enqueue_task(&init->sched_info, 0);
}


/**
 * @brief scheduler service routine
 * 
 * using the Linux Completely Fair scheduler (CFS) algorithm
 * 
 * CFS stands for "Completely Fair Scheduler," and is the new "desktop" process
 * scheduler implemented by Ingo Molnar and merged in Linux 2.6.23.  It is the
 * replacement for the previous vanilla scheduler's SCHED_OTHER interactivity
 * code.
 * 
 * 80% of CFS's design can be summed up in a single sentence: CFS basically models
 * an "ideal, precise multi-tasking CPU" on real hardware.

 * "Ideal multi-tasking CPU" is a (non-existent  :-)) CPU that has 100% physical
 * power and which can run each task at precise equal speed, in parallel, each at
 * 1/nr_running speed.  For example: if there are 2 tasks running, then it runs
 * each at 50% physical power --- i.e., actually in parallel.

 * On real hardware, we can run only a single task at once, so we have to
 * introduce the concept of "virtual runtime."  The virtual runtime of a task
 * specifies when its next timeslice would start execution on the ideal
 * multi-tasking CPU described above.  In practice, the virtual runtime of a task
 * is its actual runtime normalized to the total number of running tasks.
 * 
 *                                      Reference
 * Operating Systems: Three Easy Pieces by Remzi H. Arpaci-Dusseau and Andrea C. Arpaci-Dusseau.
 * https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-sched-lottery.pdf
 * 
 * Love, Robert, Linux Kernel Development
 * https://www.doc-developpement-durable.org/file/Projets-informatiques/cours-&-manuels-informatiques/Linux/Linux%20Kernel%20Development,%203rd%20Edition.pdf
 * 
 * Linux Documentation 
 * https://www.kernel.org/doc/Documentation/scheduler/sched-design-CFS.txt
 * 
 * External website
 * https://static.linaro.org/connect/yvr18/presentations/yvr18-220.pdf
 * 
 */
void schedule(void) {
    thread_t *curr, *next;

    /* get current thread */
    GETPRO(curr);

    /* find the next task to run */
    next = pick_next_task(&curr->sched_info);

    /* switch to the next task*/
    if (next != curr) {
        sti();
        context_switch(curr->context, next->context);
    }
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
 * @brief pick the task with the smallest vruntime
 * 
 * @param curr : the current running thread 
 * @return sched_t* : the pointer to next runable thread
 */
static thread_t *pick_next_task(sched_t *curr) {
    sched_t *next;

    /* if no task can be scheduled */
    if (!runqueue->nrunning)
        return sched;

    /* pick a new task */
    next = get_task(curr);
    
    /* return the thread */
    return task_of(next);
}


/**
 * @brief enqueue task to the runqueue
 * 
 * @param curr : current task sched info
 * @param wakeup : does the process just wake up?
 */
static void enqueue_task(sched_t *curr, int8_t wakeup) {
    /* enqueued already */
    if (curr->on_rq) return;

}


/**
 * @brief pick the next task's sched info
 * 
 * @param curr : current task sched info
 * @return sched_t* : the task's sched info
 */
static sched_t *get_task(sched_t *curr) {
    rb_tree *node = runqueue->rb_leftmost;
    return sched_of(node);
}


/**
 * @brief get the thread_t of the sched_t
 * 
 * @param s : sched_t
 * @return thread_t* : thread_t
 */
static inline thread_t *task_of(sched_t *s) {
    return (thread_t*)((void*)s - (sizeof(list_head)));
}


/**
 * @brief get the sched_t of the rb_tree
 * 
 * @param node : rb_tree
 * @return sched_t* : sched_t
 */
static inline sched_t *sched_of(rb_tree *node) {
    return (sched_t*)node;
}


/**
 * @brief update the current process's vruntime 
 * 
 * @param curr : current sched info
 * 
 * @return int32_t : 1 if need schedule 0 otherwise
 */
int32_t update_curr(sched_t *curr) {
    uint32_t now = runqueue->clock;
    uint32_t delta = now - curr->exec_time;

    /* not changed yet */
    if (!delta) return 0;

    /* update vruntime */
    curr->vruntime += delta * (NICE_0_LOAD / curr->load.weight);

    return curr->vruntime > runqueue->min_vruntime;
}


static void set_load_weight(thread_t *p) {
    weight_t *load = &p->sched_info.load;

    /* process 0*/
    if (p == sched) {
        load->weight = WEIGHT_SWAPPER;
        load->inv_weight = WMULT_SWAPPER;
        return;
    }

    /* other process */
    load->weight = sched_prio_to_weight[nice_to_index(p->nice)];
    load->inv_weight = sched_prio_to_wmult[nice_to_index(p->nice)];
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
