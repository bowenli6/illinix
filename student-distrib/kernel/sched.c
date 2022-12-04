#include <pro/process.h>
#include <pro/sched.h>
#include <kmalloc.h>
#include <access.h>
#include <drivers/vga.h>
#include <list.h>
#include <lib.h>
#include <io.h>

/* running queue */
rr_rq_t *rr_rq;



/**
 * @brief init the round-robin scheduler
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
    idle->argc = 1;
    idle->argv = kmalloc(sizeof(int8_t*));
    idle->argv[0] = kmalloc(5);
    strcpy(idle->argv[0], IDLE);
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
    init->count = TIMESLICE;
    init->argv = kmalloc(sizeof(int8_t*));
    init->argv[0] = kmalloc(5);
    strcpy(init->argv[0], INIT);
    init->context = kmalloc(sizeof(context_t));

    /* create sched run queue */
    rr_rq = kmalloc(sizeof(rr_rq_t));

    /* empty run queue */
    rr_rq->run_queue = &(idle->run_node);
    rr_rq->run_queue->prev = rr_rq->run_queue;
    rr_rq->run_queue->next = rr_rq->run_queue;
    rr_rq->size = 0;

    /* create task queue */
    task_queue = &(idle->task_node);
    task_queue->next = task_queue;
    task_queue->prev = task_queue;
    
    /* create wait queue */
    wait_queue = &(idle->task_node);
    wait_queue->next = wait_queue;
    wait_queue->prev = wait_queue;

    /* start running init */
    init->context->esp = get_esp0(init);
    asm volatile("movl %[init_esp], %%esp"
                :
                : [init_esp] "rm"(init->context->esp)
                : "memory" 
    );
    init->state = RUNNING;
    
    
    /* init should be the only process running so go to its task */
    init_task();
}



/**
 * @brief check time slices every timer tick
 * 
 */
void sched_tick(void) {
    thread_t *curr;

    GETPRO(curr);

    list_add_tail(&curr->run_node, rr_rq->run_queue);

    schedule();
}



/**
 * @brief task stopped and yield the CPU
 * 
 * @param task 
 */
void sched_sleep(thread_t *task) {
    task->state = SLEEPING;
    list_add(&task->wait_node, wait_queue);
    schedule();
}



void sched_wakeup(thread_t *task) {
    task->state = RUNNABLE;

    list_del(&task->wait_node);

    /* add to the front of the queue to get better responses */
    list_add(&task->run_node, rr_rq->run_queue);
    
    schedule();
}

/**
 * @brief scheduler service routine
 * 
 */
void schedule(void) {
    thread_t *prev, *next;
    list_head *node;

    /* avoid preemption */
    cli();

    /* get current thread struct */
    GETPRO(prev);        
    
    node = rr_rq->run_queue->next;

    list_del(node);

    next = list_entry(node, thread_t, run_node);

    /* update states */
    next->state = RUNNING;

    /* update count to start counting */
    // next->count = TIMESLICE;

    sti();
    
    if (next == prev) return;

    context_switch(prev, next);
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
