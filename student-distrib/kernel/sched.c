/**
 * @file sched.c
 * @author Bowen Li (bowenli6@illinois.edu)
 * @brief Implmentation of the Linux Completely Fair Scheduler (CFS).
 * @overview:
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
 * @reference:
 * Operating Systems: Three Easy Pieces by Remzi H. Arpaci-Dusseau and Andrea C. Arpaci-Dusseau.
 * https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-sched-lottery.pdf
 * 
 * Love, Robert, Linux Kernel Development
 * https://www.doc-developpement-durable.org/file/Projets-informatiques/cours-&-manuels-informatiques/Linux/Linux%20Kernel%20Development,%203rd%20Edition.pdf
 * 
 * Linux Documentation 
 * https://www.kernel.org/doc/Documentation/scheduler/sched-design-CFS.txt
 *   
 * Red-Black tree
 * https://github.com/torvalds/linux/blob/master/Documentation/core-api/rbtree.rst
 * 
 * External website
 * https://trepo.tuni.fi/bitstream/handle/10024/96864/GRADU-1428493916.pdf
 * https://static.linaro.org/connect/yvr18/presentations/yvr18-220.pdf
 * 
 * @version 0.9
 * @date 2022-11-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <pro/sched.h>
#include <pro/process.h>
#include <access.h>
#include <kmalloc.h>
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


static thread_t *pick_next_task(sched_t *curr);
static sched_t *pick_next_entity(void);
static void put_prev_task(sched_t *prev);
static void dequeue_task(thread_t *prev, int8_t sleep);
static void dequeue_entity(sched_t *prev, int8_t sleep);
static void __dequeue_entity(sched_t *s);
static void enqueue_task(thread_t *new, int8_t wakeup);
static void enqueue_entity(sched_t *s, int8_t wakeup);
static void __enqueue_entity(sched_t *s);
static int32_t check_preempt_new(sched_t *curr, sched_t *new);
static int32_t check_preempt_tick(sched_t *curr);
static void update_curr(void);
static void update_min_vruntime(void);
static inline uint64_t calc_delta_vruntime(uint64_t delta, sched_t *s);
static uint64_t __calc_delta_vruntime(uint64_t delta, uint32_t weight, weight_t *load);
static inline uint64_t mul_u64_u32_shr(uint64_t a, uint32_t mul, uint32_t shift);
static inline uint64_t max_vruntime(uint64_t min_vruntime, uint64_t vruntime);
static inline uint64_t min_vruntime(uint64_t min_vruntime, uint64_t vruntime);
static inline void set_load_weight(sched_t *s, int32_t nice);
static void place_entity(sched_t *s, int8_t new_task);
static uint64_t vtimeslice(sched_t *s);
static uint64_t timeslice(sched_t *s);
static uint64_t sched_period(uint32_t nr_running);
static inline void add_load(weight_t *load, uint32_t weight);
static inline void sub_load(weight_t *load, uint32_t weight);
static inline int32_t nice_to_index(int32_t nice);


/**
 * @brief init the scheduler (process 0)
 * 
 */
void sched_init(void) {
    /* allocate memory spaces for kernel threads */
    process_t *idlep = (process_t *) alloc_kstack();
    process_t *initp = (process_t *) alloc_kstack();

    /* set up process 0 */
    idle = &idlep->thread;
    idle->pid = 0;
    idle->state = RUNNABLE;
    idle->parent = NULL;
    idle->kthread = 1;
    idle->sched_info.load.weight = WEIGHT_IDLE;
    idle->sched_info.load.inv_weight = WMULT_IDLE;
    idle->context = kmalloc(sizeof(context_t));
    
    /* set up process 1 */
    init = &initp->thread;
    init->pid = 1;
    init->state = RUNNABLE;  
    init->parent = idle;
    init->n_children = 0;
    init->children = NULL;
    init->max_children = MAXCHILDREN;
    init->nice = NICE_INIT;
    init->kthread = 1;
    init->argc = 1;
    init->argv = kmalloc(sizeof(int8_t*));
    init->argv[0] = kmalloc(5);
    strcpy(init->argv[0], INIT);
    init->context = kmalloc(sizeof(context_t));
    
    /* create task queue */
    task_queue = &(idle->task_node);
    task_queue->next = task_queue;
    task_queue->prev = task_queue;
    list_add(&(init->task_node), task_queue);
    
    /* create wait queue */
    wait_queue = &(idle->task_node);
    wait_queue->next = wait_queue;
    wait_queue->prev = wait_queue;
    
    /* create run queue */
    rq = kmalloc(sizeof(cfs_rq));
    rq->nr_running = 0;
    rq->current = NULL;
    rq->left_most = NULL;
    
    // rq->root = ?

    /* add init process to the run queue */
    // sched_fork(init);
}


/**
 * @brief set up a new task (or wakeup task) for scheduling
 * 
 * @param new : new task thread info
 */
void sched_fork(thread_t *task) {
    sched_t *curr = rq->current;
    sched_t *new = &task->sched_info;

    /* set weights */
    set_load_weight(new, task->nice);

    if (curr) {
        /* update vruntime of the current process */
        update_curr();

        /* new task first get vruntime from its parent */
        new->vruntime = curr->vruntime;  
    }

    /* set vruntime for new task */
    place_entity(new, 1);

    /* new task become runnable */
    task->state = RUNNABLE;

    /* enqueue task to runqueue */
    enqueue_task(task, 0);

    /* check if reschedling is needed */
    if (check_preempt_new(curr, new) == 1) task->flag = NEED_RESCHED;
}


/**
 * @brief invoked when a thread is exited
 * 
 */
void sched_exit(void) {
    sched_t *next;
    thread_t *torun;

    /* get next sched entity */
    next = pick_next_entity();

    /* remove the picked task from the run queue */
    __dequeue_entity(next);

    rq->current = next;

    next->exec_start = rq->clock;

    next->prev_sum_exec_time = next->sum_exec_time;
    
    torun = task_of(next);

    torun->state = RUNNING;

    // context_switch(NULL, torun);
}


/**
 * @brief wake up a sleeping process
 * 
 * @param task : task info
 */
void sched_wakeup(thread_t *task) {
    task->state = RUNNABLE;
    enqueue_task(task, 1);
}



/**
 * @brief let a running process to sleep
 * 
 * @param task : task info 
 */
void sched_sleep(thread_t *task) {
    task->state = SLEEPING;
    schedule();
}


/**
 * @brief scheduler service routine
 * where to call this rountime:
 * (1). timer interrupt (1ms) will check NEED_RESCHED flag
 *  if NEED_RESCHED == 1, schedule()
 * 
 * (2). process wants to sleep, schedule()
 */
void schedule(void) {
    thread_t *curr, *next; 
    sched_t *sched;
    uint32_t flags;

    /* avoid preemption */
    cli_and_save(flags);

    /* get current thread */
    GETPRO(curr);

    /* clear NEED_RESCHED flag */
    curr->flag = 0;

    /* get sched info of the current task */
    sched = &curr->sched_info;

    /* if the current state has been sleeping or has been stopped */
    if (curr->state == SLEEPING) {
        /* remove the task from the runqueue */
        dequeue_task(curr, 1);
        sched->on_rq = 0;
    }

    /* find the next task to run */
    next = pick_next_task(sched);

    /* switch to the next task*/
	if (likely(curr != next)) {

        if (curr->state == RUNNING) 
            curr->state = RUNNABLE;

        next->state = RUNNING;
        rq->current = &next->sched_info;

        restore_flags(flags);
        context_switch(curr, next);
        return;
    }

    restore_flags(flags);
}



void __schedule(void) {
    uint32_t flags;
    thread_t *curr;

     /* avoid preemption */
    cli_and_save(flags);

    /* get current thread */
    GETPRO(curr);

    if (curr->state == RUNNING) 
        curr->state = RUNNABLE;

    curr->children[0]->state = RUNNING;

    context_switch(curr, curr->children[0]);
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
    if (!rq->nr_running)
        return idle;

    /* store the current task back to the run queue only if curr is present */
    put_prev_task(curr);
    
    /* get next sched entity */
    next = pick_next_entity();

    /* remove the picked task from the run queue */
    __dequeue_entity(next);

    rq->current = next;

    next->exec_start = rq->clock;

    next->prev_sum_exec_time = next->sum_exec_time;
    
    /* return the thread */
    return task_of(next);
}


/**
 * @brief pick the next sched info with the smallest vruntime
 * and (if possible) cached the next next rbnode into rq->left_most
 * and its vruntime into rq->min_vruntime
 * 
 * @return sched_t* : picked sched info
 */
static sched_t *pick_next_entity(void) {
    rb_node *__left_most = rq->left_most;

    if (unlikely(!__left_most)) return NULL;

    return sched_of(__left_most);
}


/**
 * @brief restore task into runqueue
 * 
 * @param prev : sched info
 */
static void put_prev_task(sched_t *prev) {

    /* If the prev process is still on the run queue, it is very likely 
     * that the prev process is preempted. Before giving up the cpu, it 
     * is necessary to update the process runtime and other information. 
     */
    if (prev->on_rq) {
        update_curr();
        /* cache the next entity which has the min_vruntime if have any */
        __enqueue_entity(prev);
    }

    rq->current = NULL;
}


/**
 * @brief remove a prev task from runqueue
 * 
 * @param new : prev task thread info
 * @param sleep : does the process just sleep?
 */
static void dequeue_task(thread_t *prev, int8_t sleep) {
    sched_t *s = &prev->sched_info;
    dequeue_entity(s, sleep);
}


/**
 * @brief called when a task enters a sleep state. 
 * It remove the scheduling entity (task) from the red-black 
 * tree and decrements the nr_running variable.
 * 
 * @param s : a scheduling entity
 * @param sleep : does the process just sleep?
 */
static void dequeue_entity(sched_t *prev, int8_t sleep) {
    update_curr();

    if (prev != rq->current)
        __dequeue_entity(prev);
    
    sub_load(&rq->load, prev->load.weight);
    rq->nr_running--;
    prev->on_rq = 0;
}


/**
 * @brief remove s from rea-black tree
 * 
 * @param s : sched info to remove
 */
static void __dequeue_entity(sched_t *s) {
    
    // rb_remove(s->node);
}



/**
 * @brief add a new task (or wakeup task) to runqueue
 * 
 * @param new : new task thread info
 * @param wakeup : does the process just wake up?
 */
static void enqueue_task(thread_t *new, int8_t wakeup) {
    sched_t *s = &new->sched_info;
    
    if (s->on_rq) return;

    enqueue_entity(s, wakeup);
}


/**
 * @brief called when a task enters a runnable state. 
 * It puts the scheduling entity (task) into the red-black 
 * tree and increments the nr_running variable.
 * 
 * @param s : a scheduling entity
 * @param wakeup : does the task just wake up?
 */
static void enqueue_entity(sched_t *s, int8_t wakeup) {
    /* update sum of all runnable tasks' load weights */
    add_load(&rq->load, s->load.weight);

    if (wakeup) {
        /* remove it from the wait_queue */
        list_del(&(task_of(s)->task_node));

        /* adjust vruntime */
        place_entity(s, 0);
    }

    /* add to run queue */
    if (rq->current != s)
        __enqueue_entity(s);

    s->on_rq = 1;
    rq->nr_running++;
}


/**
 * @brief add s to rea-black tree using its vruntime as key
 * 
 * @param s : sched info to add
 */
static void __enqueue_entity(sched_t *s) {
    // rb_add(&s->node, s->vruntime - rq->min_vruntime);
}


/**
 * @brief check if new task can preempt the current task
 * 
 * @param curr : current task
 * @param check : new task
 * @return int32_t : 1 for yes, -1 for no, 0 for maybe, but don't want to
 * (because their vruntime are really close, we don't want too many context switches)
 */
static int32_t check_preempt_new(sched_t *curr, sched_t *new) {
    int64_t delta = curr->vruntime - new->vruntime;
    if (delta <= 0)
        return -1;

    if (delta > WAKEUP_GRANULARITY) 
        return 1;

    return 0;
}


/**
 * @brief called everytime when a timer interrupt is fired
 * 
 * @param curr : current sched info
 */
void task_tick(sched_t *curr) {
    /* update vruntime of the current task */
    update_curr();
    
    /* only try to reschedule when there are more than 1 runnable task */
    if (rq->nr_running) {
        if (check_preempt_tick(curr) == 1) {
            task_of(curr)->flag = NEED_RESCHED;
        }
    }
}


/**
 * @brief check if there is a task can preempt the curren task
 * 
 * @param curr : current task sched info
 * @return int32_t : 1 to reschedule, 0 otherwise
 */
static int32_t check_preempt_tick(sched_t *curr) {
    uint64_t ideal = timeslice(curr);
    uint64_t delta = curr->sum_exec_time - curr->prev_sum_exec_time;

    /* has used all timeslice: should be preempted */
    if (delta > ideal) 
        return 1;
    
    /* in order to prevent frequent excessive preemption, we should 
     * ensure that the running time of each process should not be less 
     * than the minimum granularity time. So if the running time is 
     * less than the minimum granularity time, it should not be preempted. 
     */
    if (delta < MIN_GRANULARITY)
        return 0;
    
	delta = curr->vruntime - rq->min_vruntime;
 
    /* vruntime of the current process is still smaller than the vruntime 
     * of the leftmost scheduling entity in the red-black tree */
	if (delta < 0) return 0;
 
    /* NOT A BUG. 
     * After reviewing the submission records, the author's 
     * intention is: I hope that tasks with small weights will be preempted 
     * more easily. Ensure that a task that missed wakeup preemption by a
     * narrow margin doesn't have to wait for a full slice. This also mitigates 
     * buddy induced latencies under load.
     * (MAGIC CODE BUT SOMEHOW WORKS!)
     */
	if (delta > ideal) return 1;

    return 0;
}   


/**
 * @brief update the current process's vruntime 
 * 
 */
static void update_curr(void) {
    sched_t *curr = rq->current;
    uint64_t now = rq->clock;
    uint64_t delta;

    if (unlikely(!curr)) return;    
    
    /* calculate the runtime difference since last scheduled */
    delta = now - curr->exec_start;

    if (unlikely((int64_t)delta <= 0)) return;

    /* update total runtime */
    curr->sum_exec_time += delta;  
    
    /* start change to now */
    curr->exec_start = now;

    /* update vruntime */
    curr->vruntime += calc_delta_vruntime(delta, curr);  

    /* update min_vruntime */
    update_min_vruntime();
}


/**
 * @brief update current min_vruntime
 * 
 */
static void update_min_vruntime(void) {
    sched_t *s;
    uint64_t vruntime = rq->min_vruntime;
    
    if (rq->current) vruntime = rq->current->vruntime;

    /* if there is a cached left most node in the queue */
    if (rq->left_most) {
        /* get the sched info of the node */
        s = sched_of(rq->left_most);

        /* update min_vruntime */
        if (!rq->current) vruntime = s->vruntime;
        else vruntime = min_vruntime(vruntime, s->vruntime);
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
	if (unlikely(s->load.weight != NICE_0_LOAD)) {
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
 * @param weight : weight offset
 * @param load : load weight of the thread
 * @return uint64_t : amount of changes of the virtual runtime (nanosecond)
 */
static uint64_t __calc_delta_vruntime(uint64_t delta, uint32_t weight, weight_t *load) {
    uint64_t fact = weight;
    int32_t shift = WMULT_SHIFT;

    if (unlikely(fact >> 32)) {
		while (fact >> 32) {
			fact >>= 1;
			shift--;
		}
	}

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
static inline void set_load_weight(sched_t *s, int32_t nice) {
    weight_t *load = &s->load;

    load->weight = sched_prio_to_weight[nice_to_index(nice)];
    load->inv_weight = sched_prio_to_wmult[nice_to_index(nice)];
}


/**
 * @brief adjust vruntime for the task
 * 
 * @param s : sched info
 * @param new_task : is the task a new task?
 */
static void place_entity(sched_t *s, int8_t new_task) {
    uint64_t vruntime = rq->min_vruntime;

    if (new_task) {
        /* punish new task */
        vruntime += vtimeslice(s);
    } else {
        /* make up for sleeping task */
        vruntime -= (TARGET_LATENCY >> 1);
    }

    /* avoid some process that only sleep for a short period of time to get compensation
     * but..WHY? consider this: 
     * When a process has just slept for 1ms, and when it woke up, 
     * you decide to award it 3ms by reducing its vruntime from 3ms, 
     * so it then profit 2ms. We want to be FAIR! */
    s->vruntime = max_vruntime(s->vruntime, vruntime);
}

/**
 * @brief assign s its virtual time slice
 * 
 * @param s : sched info
 * @return uint64_t : virtual time slice
 */
static uint64_t vtimeslice(sched_t *s) {
    return calc_delta_vruntime(timeslice(s), s);
}


/**
 * @brief assign s its real time slice
 * 
 * @param s : sched info
 * @return uint64_t : real time slice
 */
static uint64_t timeslice(sched_t *s) {
	uint64_t slice = sched_period(rq->nr_running + !s->on_rq);
    weight_t load = rq->load;

    /* get the current sum of weights */
    weight_t *rq_load = &rq->load;

	if (unlikely(!s->on_rq)) {
        /* if (unlikely) s is not on the runqueue
         * use a new load to include s */
        add_load(&load, s->load.weight);
        rq_load = &load;
    }

    /* calculate the proportion of the weight of the scheduling entity se 
     * to the weight of the entire ready queue, and then multiply it by 
     * the scheduling cycle time to get the time that the current scheduling 
     * entity should run. */
    return __calc_delta_vruntime(slice, s->load.weight, rq_load);
}


/**
 * @brief get the total timeslices
 * 
 * @param nr_running : number of runnable task
 * @return uint64_t : total timeslices
 */
static uint64_t sched_period(uint32_t nr_running) {
        uint64_t period = TARGET_LATENCY;
        
        if (unlikely(nr_running > NR_LATENCY)) {
                period = MIN_GRANULARITY;
                period *= nr_running;
        }

        return period;
}



/**
 * @brief update load weight by adding weight
 * 
 * @param load : load weight
 * @param weight : offset
 */
static inline void add_load(weight_t *load, uint32_t weight) {
    load->weight += weight;
    load->inv_weight += weight;
}


/**
 * @brief update load weight by subtracting weight
 * 
 * @param load : load weight
 * @param weight : offset
 */
static inline void sub_load(weight_t *load, uint32_t weight) {
    load->weight -= weight;
    load->inv_weight -= weight;
}


/**
 * @brief nice[-20, 19] => index[0, 39]
 * 
 * @param nice : nice value 
 * @return int8_t : index to the sched_prio_to_x array
 */
static inline int32_t nice_to_index(int32_t nice) {
    return nice + 20;
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
