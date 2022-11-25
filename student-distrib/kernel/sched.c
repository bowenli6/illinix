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
cfs_rq *runqueue;


/* local helper functions */

static inline int8_t nice_to_index(int8_t nice);
static thread_t *pick_next_task(sched_t *curr);
static void place_entity(sched_t *s, int8_t new_task);
static inline thread_t *task_of(sched_t *s);
static inline sched_t *sched_of(rb_tree *node);
static inline void set_load_weight(sched_t *s, int8_t nice);
static void sleep(sched_t *curr);
static inline uint32_t alloc_timeslice(sched_t *s);
static inline uint32_t alloc_vruntime(sched_t *s);
static inline uint32_t max_vruntime(uint32_t min_vruntime, uint32_t vruntime);
static inline uint32_t min_vruntime(uint32_t min_vruntime, uint32_t vruntime);



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
    init->kthread = 1;   
    init->argc = 1;
    init->sched_info.nice = 5;
    strcpy(init->argv, "init");
    
    /* create task queue */
    task_head = &(idle->task);
    task_head->next = task_head;
    task_head->prev = task_head;
    list_add(&(init->task), task_head);

    /* create runqueue */
    runqueue = kmalloc(sizeof(cfs_rq));
    runqueue->clock = 0;
    runqueue->nrunning = 0;

    /* create wait queue */
    

    /* add init process to the run queue */
    place_entity(&init->sched_info, 1);
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
        runqueue->current = &next->sched_info;
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
    if (curr->on_rq) {
        update_curr(curr);
        enqueue_entity(curr, 0);
    }
    runqueue->current = NULL;
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
    if (!runqueue->nrunning)
        return idle;
    
    next = runqueue->left_most;

    /* remove the leftmost node from queue */
    if (!(nnext = dequeue_entity(runqueue->left_most))) {
        runqueue->nrunning = 0;
        runqueue->current = runqueue->left_most;
        runqueue->left_most = NULL; 
        runqueue->min_vruntime = 0;
    } else {
        runqueue->nrunning--;
        runqueue->current = runqueue->left_most;
        runqueue->left_most = nnext;
        runqueue->min_vruntime = nnext->vruntime;
    }
    
    /* return the thread */
    return task_of(rb_entry(next));
}


static void enqueue_entity(sched_t *s, int8_t wakeup) {
    if (wakeup) {
        /* update its vruntime */
        s->vruntime += runqueue->min_vruntime;

        /* remove it from the wait_queue */
        list_del(&s->wait_node);
        place_entity(s, 0);
    }

    if (s != runqueue->current) {
        __enqueue_entity(s);
    }
    s->on_rq = 1;
}


static void __enqueue_entity(sched_t *s) {
    add_rbnode(&s->node, s->vruntime - runqueue->min_vruntime);
}


/**
 * @brief set_up task when creating
 * 
 * @param new : new task sched info
 */
void set_sched_task(sched_t *new) {
    sched_t *curr = runqueue->current;

    /* enqueued already */
    if (new->on_rq) return;

    /* set weights */
    set_load_weight(new, new->nice);

    /* set time slice*/
    new->timeslice = alloc_timeslice(new);

    new->exec_time = runqueue->clock;

    if (curr) {
        /* update vruntime of the current process */
        (void) update_curr(curr);
        new->vruntime = curr->vruntime;  /* new task first get vruntime from its parent */
    }

    /* set vruntime for new task */
    place_entity(new, 1);

    /* put curr task into runqueue */
    sleep(curr);
}


static int32_t wakeup_preempt_entity(sched_t *curr, sched_t *check) {
    int32_t delta = curr->vruntime - check->vruntime;
    return (delta > 0) && (delta > WAKEUP_GRANULARITY); 
}


/**
 * @brief adjust vruntime for the task
 * 
 * @param s 
 * @param new_task : 1 if this is a new task, 0 otherwise
 */
static void place_entity(sched_t *s, int8_t new_task) {
    uint32_t vruntime = runqueue->min_vruntime;

    if (new_task) {
        /* update vruntime for new task */
        vruntime += alloc_vruntime(s);
    } else {
        /* make up for sleeping process */
        vruntime -= (TARGET_LATENCT >> 1);
    }

    /* avoid some process that only sleep for a short period of time to get compensation */
    s->vruntime = max_vruntime(s->vruntime, vruntime);
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


static void sleep(sched_t *curr) {
    dequeue_entity(curr);
    list_add(&task_of(curr)->task, wait_queue);
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
    uint32_t delta;
    
    if (!curr) return 0; 

    delta = now - curr->exec_time;

    /* should not schedule now */
    if (delta < curr->timeslice) return 0;

    curr->exec_time = now;

    /* update vruntime */
    curr->vruntime += delta * (NICE_0_LOAD / curr->load.weight);

    return min_vruntime(curr->vruntime, runqueue->min_vruntime) == runqueue->min_vruntime;
}



/**
 * @brief find the max vruntime of two vruntime
 * 
 */
static inline uint32_t max_vruntime(uint32_t min_vruntime, uint32_t vruntime) {
	int32_t delta = (int32_t)(vruntime - min_vruntime);
	if (delta > 0)
		min_vruntime = vruntime;

	return min_vruntime;
}


/**
 * @brief find the min vruntime of two vruntime
 * 
 */
static inline uint32_t min_vruntime(uint32_t min_vruntime, uint32_t vruntime) {
	int32_t delta = (int32_t)(vruntime - min_vruntime);
	if (delta < 0)
		min_vruntime = vruntime;

	return min_vruntime;
}


static inline void set_load_weight(sched_t *s, int8_t nice) {
    weight_t *load = &s->load;

    load->weight = sched_prio_to_weight[nice_to_index(nice)];
    load->inv_weight = sched_prio_to_wmult[nice_to_index(nice)];
}


static inline uint32_t alloc_timeslice(sched_t *s) {
    return ((s->load.weight / runqueue->total_weights) * TARGET_LATENCT, MIN_GRANULARITY);
}


static inline uint32_t alloc_vruntime(sched_t *s) {
    return alloc_timeslice(s) * (NICE_0_LOAD / s->load.weight);
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
