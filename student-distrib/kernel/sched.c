#include <pro/sched.h>
#include <pro/process.h>
#include <access.h>
#include <lib.h>


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

    /* set up process 1 */
    init = &initp->thread;
    init->state = RUNNABLE;  
    init->parent = sched;
    init->child = NULL;
    init->kthread = 1;   

    /* start to running the task of process 0 */
    swapper();
}


/**
 * @brief scheduler service routine
 * 
 * @return int32_t : 0 only if there are no tasks can be scheduled
 */
int32_t schedule(void) {
    /* naive implmentation of a scheduler */

    thread_t *thread;
    console_t *console;

    /* avoid deadlock by ensuring that devices can interrupt */
    sti();

    /* init start running and save its context */
    if (init->state == RUNNABLE) 
        init_task();

    for (thread = tasks; thread < &tasks[NTASK]; thread++) {
        if (thread->state == RUNNABLE) {
            /* ready to run the chosen thread */
            thread->state = RUNNING;
            curr_pid = thread->pid;

            /* copy the current hardware context and switch to the chosen thread */
            context_switch(sched->context, thread->context);
        }
    }

    /* no task can be scheduled 
     * return, pause, and wait for 
     * awaking by a timer interrupt */
    return 0;
}





/**
 * @brief halts the central processing unit (CPU) until 
 * the next external interrupt is fired.
 * 
 */
void pause(void) {
    sti(); /* enable interrupts */

    /* Spin (nicely, so we don't chew up cycles) */
    asm volatile (".1: hlt; jmp .1;");
}
