#include <pro/sched.h>
#include <pro/process.h>
#include <access.h>
#include <lib.h>

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
cfs_rq *rq;


/* local helper functions */

static inline int8_t nice_to_index(int8_t nice);
static thread_t *pick_next_task(sched_t *curr);
static void sleep(sched_t *curr);

static void update_min_vruntime(void);
static inline uint64_t calc_delta_vruntime(uint64_t delta, sched_t *s);
static uint64_t __calc_delta_vruntime(uint64_t delta, uint32_t weight, weight_t *load);
static inline uint64_t mul_u64_u32_shr(uint64_t a, uint32_t mul, uint32_t shift);
static inline uint64_t max_vruntime(uint64_t min_vruntime, uint64_t vruntime);
static inline uint64_t min_vruntime(uint64_t min_vruntime, uint64_t vruntime);
static inline void set_load_weight(sched_t *s, int8_t nice);
static void place_entity(sched_t *s, int8_t new_task);



/**
 * @brief init the scheduler (process 0)
 * 
 */
void sched_init(void) {
    /* allocate memory spaces for kernel threads */
    process_t *idlep = (process_t *) alloc_kstack(0);
    process_t *initp = (process_t *) alloc_kstack(1);

    /* set up process 0 */
    idle = &idlep->thread;
    idle->state = RUNNABLE;
    idle->parent = NULL;
    idle->child = init;
    idle->kthread = 1;
    idle->argc = 1;
    strcpy(idle->argv, "idle");
    
    /* set up process 1 */
    init = &initp->thread;
    init->state = RUNNABLE;  
    init->parent = idle;
    init->child = NULL;
    init->nice = 5;
    init->kthread = 1;
    init->argc = 1;
    strcpy(init->argv, "init");
    
    /* create task queue */
    task_head = &(idle->task);
    task_head->next = task_head;
    task_head->prev = task_head;
    list_add(&(init->task), task_head);

    /* create runqueue */
    rq = kmalloc(sizeof(cfs_rq));
    rq->nr_running = 0;
    rq->current = NULL;
    rq->left_most = NULL;
    // rq->root = ?
    
    /* create wait queue */
    wait_queue

    /* add init process to the run queue */
    place_entity(&init->sched_info, 1);
    enqueue_task(&init->sched_info, 0);
}


/**
 * @brief scheduler service routine
 * 
 * using the Linux Completely Fair Scheduler (CFS) algorithm
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
 * https://trepo.tuni.fi/bitstream/handle/10024/96864/GRADU-1428493916.pdf
 * https://static.linaro.org/connect/yvr18/presentations/yvr18-220.pdf
 * 
 */
void schedule(void) {
    thread_t *curr, *next; 
    uint32_t flags;

    /* avoid preemption */
    cli_and_save(flags);

    /* get current thread */
    GETPRO(curr);

    /* store the current task */
    put_prev_task(&curr->sched_info);

    /* find the next task to run */
    next = pick_next_task(&curr->sched_info);

    /* switch to the next task*/
    if (next != curr) {
        curr->state = RUNNABLE;
        next->state = RUNNING;
        rq->current = &next->sched_info;
        restore_flags(flags);
        context_switch(curr->context, next->context);
        return;
    }

    restore_flags(flags);
}



/**
 * @brief nice[-20, 19] => index[0, 39]
 * 
 * @param nice : nice value 
 * @return int8_t : index to the sched_prio_to_x array
 */
static inline int8_t nice_to_index(int8_t nice) {
    return nice + 20;
}


static void put_prev_task(sched_t *curr) {
    if (!curr->on_rq) {
        update_curr(curr);
        enqueue_entity(curr, 0);
    }
    rq->current = NULL;
}


/**
 * @brief pick the task with the smallest vruntime
 * 
 * @param curr : the current running thread 
 * @return sched_t* : the pointer to next runable thread
 */
static thread_t *pick_next_task(sched_t *curr) {
    sched_t *next, *nnext;

    /* if no task can be scheduled */
    if (!rq->nr_running)
        return idle;
    
    next = rq->left_most;

    /* remove the leftmost node from queue */
    if (!(nnext = dequeue_entity(rq->left_most))) {
        rq->nr_running = 0;
        rq->current = rq->left_most;
        rq->left_most = NULL; 
        rq->min_vruntime = 0;
    } else {
        rq->nrunning--;
        rq->current = rq->left_most;
        rq->left_most = nnext;
        rq->min_vruntime = nnext->vruntime;
    }
    
    /* return the thread */
    return task_of(rb_entry(next));
}


/**
 * @brief Called when a task enters a runnable state. 
 * It puts the scheduling entity (task) into the red-black 
 * tree and increments the nr_running variable.
 * 
 * @param s : a scheduling entity
 * @param wakeup : does the task just wake up?
 */
static void enqueue_entity(sched_t *s, int8_t wakeup) {
    if (wakeup) {
        /* update its vruntime */
        s->vruntime += rq->min_vruntime;

        /* remove it from the wait_queue */
        list_del(&s->wait_node);
        place_entity(s, 0);
    }

    if (s != rq->current) {
        __enqueue_entity(s);
    }
    s->on_rq = 1;
}


static void __enqueue_entity(sched_t *s) {
    add_rbnode(&s->node, s->vruntime - rq->min_vruntime);
}


/**
 * @brief set_up task when creating
 * 
 * @param new : new task 
 * @param 
 */
void set_sched_task(thread_t *new, int8_t wakeup) {
    sched_t *curr = rq->current;
    sched_t *s = &new->sched_info;
    /* enqueued already */
    if (s->on_rq) return;

    /* set nice value */
    new->nice = NICE_NORMAL;

    /* set weights */
    set_load_weight(new, new->nice);

    if (curr) {
        /* update vruntime of the current process */
        update_curr(curr);
        s->vruntime = curr->vruntime;  /* new task first get vruntime from its parent */
    }

    /* set vruntime for new task */
    place_entity(new, 1);

    /* put curr task into wait_queue */
    s->vruntime -= rq->min_vruntime;

    new->state = RUNNABLE;
    enqueue_task(s, );
}

static void enqueue_task(sched_t *s, int8_t wakeup) {

}


static int32_t wakeup_preempt_entity(sched_t *curr, sched_t *check) {
    int32_t delta = curr->vruntime - check->vruntime;
    return (delta > 0) && (delta > WAKEUP_GRANULARITY); 
}


/**
 * @brief update the current process's vruntime 
 * 
 * @param curr : current sched info
 */
void update_curr(sched_t *curr) {
    uint64_t now = rq->clock;
    uint32_t delta;
    
    delta = (uint32_t) (now - curr->exec_start);

    if (!delta) return;

    /* update total runtime */
    curr->sum_exec_time += delta;  
    
    /* start change to now */
    curr->exec_start = now;

    /* update vruntime */
    curr->vruntime += calc_delta_vruntime(delta, curr);  

    /* update min_vruntime */
    update_min_vruntime();
}


static void update_min_vruntime(void) {
    sched_t *s;
    uint64_t vruntime = rq->min_vruntime;
    
    /* if there is a cached left most node in the queue */
    if (rq->left_most) {
        /* get the sched info of the node */
        s = rb_entry(rq->left_most, sched_t, node);

        /* update min_vruntime */
        if (!rq->current) vruntime = s->vruntime;
        else vruntime = min_vruntime(rq->current->vruntime, s->vruntime);
    }

    /* ensure we never gain time by being placed backwards */
    rq->min_vruntime = max_vruntime(rq->min_vruntime, vruntime);
}


/**
 * @brief map real runtime to virtual runtime
 * 
 * @param delta : amount of changes of the real runtime (nanosecond)
 * @param s : sched info
 * @return uint64_t : amount of changes of the virtual runtime (nanosecond)
 */
static inline uint64_t calc_delta_vruntime(uint64_t delta, sched_t *s) {
    /* if the task has nice value different than 0 */
    if (s->load.weight != NICE_0_LOAD) {
        delta = __calc_delta_vruntime(delta, NICE_0_LOAD, &s->load);
    }

    /* otherwise the real runtime is the virtual runtime */
    return delta;
}


/**
 * @brief helper function to map real runtime to virtual runtime
 * 
 * vruntime = (delta * NICE_0_LOAD) / weight
 * 
 * in order to avoid floating point (divide) arithmetic, we first shift left 32 bits, 
 * and then shift right 32 bits.
 * hence: 
 * vruntime = ((delta * NICE_0_LOAD) * (2 ** 32 / weight)) >> 32
 *          = (delta * NICE_0_LOAD * inv_weight) >> 32
 * inv_weight = (2 ** 32) / weight
 * 
 * @param delta : amount of changes of the real runtime (nanosecond)
 * @param weight : NICE_0_LOAD
 * @param load : load weight of the thread
 * @return uint64_t : amount of changes of the virtual runtime (nanosecond)
 */
static uint64_t __calc_delta_vruntime(uint64_t delta, uint32_t weight, weight_t *load) {
    uint64_t fact = weight;
    int32_t shift = WMULT_SHIFT;

    /* NICE_0_LOAD * inv_weight */
    fact = (uint64_t)((uint32_t) fact * load->inv_weight);

    while (fact >> 32) {
        fact >>= 1;
        shift--;
    }
    
    return mul_u64_u32_shr(delta, fact, shift);
}


/**
 * @brief multply a u64 number with u32 number and a shift
 * 
 * @param a : u64 number
 * @param mul : u32 number
 * @param shift : shift number
 * @return uint64_t : result
 */
static inline uint64_t mul_u64_u32_shr(uint64_t a, uint32_t mul, uint32_t shift) {
	uint32_t ah, al;
	uint64_t ret;

	al = a;
	ah = a >> 32;

	ret = ((uint64_t)al * mul) >> shift;
	if (ah)
		ret += ((uint64_t)ah * mul) << (32 - shift);

	return ret;
}


/**
 * @brief find the max vruntime of two vruntime
 * 
 */
static inline uint64_t max_vruntime(uint64_t min_vruntime, uint64_t vruntime) {
	int64_t delta = (int64_t)(vruntime - min_vruntime);
	if (delta > 0)
		min_vruntime = vruntime;

	return min_vruntime;
}


/**
 * @brief find the min vruntime of two vruntime
 * 
 */
static inline uint64_t min_vruntime(uint64_t min_vruntime, uint64_t vruntime) {
	int64_t delta = (int64_t)(vruntime - min_vruntime);
	if (delta < 0)
		min_vruntime = vruntime;

	return min_vruntime;
}


/**
 * @brief Set the load weight for a tsak
 * 
 * @param s : sched info
 * @param nice : nice value
 */
static inline void set_load_weight(sched_t *s, int8_t nice) {
    weight_t *load = &s->load;

    load->weight = sched_prio_to_weight[nice_to_index(nice)];
    load->inv_weight = sched_prio_to_wmult[nice_to_index(nice)];
}


/**
 * @brief adjust vruntime for the task
 * 
 * @param s : sched info
 * @param new_task : 1 if this is a new task, 0 otherwise
 */
static void place_entity(sched_t *s, int8_t new_task) {
    uint32_t vruntime = rq->min_vruntime;

    if (new_task) {
        /* punish new task */
        vruntime += vslice(s);
    } else {
        /* make up for sleeping task */
        vruntime -= (TARGET_LATENCT >> 1);
    }

    /* avoid some process that only sleep for a short period of time to get compensation */
    s->vruntime = max_vruntime(s->vruntime, vruntime);
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
