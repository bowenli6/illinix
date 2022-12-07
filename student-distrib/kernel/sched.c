// #include <pro/process.h>
// #include <pro/sched.h>
// #include <kmalloc.h>
// #include <access.h>
// #include <drivers/vga.h>
// #include <list.h>
// #include <lib.h>
// #include <io.h>

// /* running queue */
// rr_t *rq;


// /**
//  * @brief init the round-robin scheduler
//  * 
//  */
// void sched_init(void) {
//     /* allocate memory spaces for kernel threads */
//     process_t *idlep = (process_t *) alloc_kstack();
//     process_t *initp = (process_t *) alloc_kstack();

//     /* set up process 0 */
//     idle = &idlep->thread;
//     idle->pid = 0;
//     idle->state = RUNNABLE;
//     idle->parent = NULL;
//     idle->kthread = 1;
//     idle->argc = 1;
//     idle->argv = kmalloc(sizeof(int8_t*));
//     idle->argv[0] = kmalloc(5);
//     strcpy(idle->argv[0], IDLE);
//     idle->context = kmalloc(sizeof(context_t));
    
//     /* set up process 1 */
//     init = &initp->thread;
//     init->pid = 1;
//     init->state = RUNNABLE;  
//     init->parent = idle;
//     init->n_children = 0;
//     init->children = NULL;
//     init->max_children = MAXCHILDREN;
//     init->nice = NICE_INIT;
//     init->kthread = 1;
//     init->argc = 1;
//     init->count = TIMESLICE;
//     init->argv = kmalloc(sizeof(int8_t*));
//     init->argv[0] = kmalloc(5);
//     strcpy(init->argv[0], INIT);
//     init->context = kmalloc(sizeof(context_t));

//     /* create run queue */
//     rq = kmalloc(sizeof(rr_t));
//     rq->head.next = &(rq->head);
//     rq->head.prev = &(rq->head);
//     rq->size = 0;

//     /* create task queue */
//     task_queue.next = &task_queue;
//     task_queue.prev = &task_queue;
    
//     /* create wait queue */
//     wait_queue.next = &wait_queue;
//     wait_queue.prev = &wait_queue;

//     /* create console queue */
//     consoles = kmalloc(NTERMINAL * sizeof(console_t));
//     current = NULL;

//     /* start running init */
//     init->context->esp = get_esp0(init);
//     asm volatile("movl %[init_esp], %%esp"
//                 :
//                 : [init_esp] "rm"(init->context->esp)
//                 : "memory" 
//     );
//     init->state = RUNNING;
    
    
//     /* init should be the only process running so go to its task */
//     init_task();
// }



// /**
//  * @brief check time slices every timer tick
//  * 
//  */
// void sched_tick(void) {
//     thread_t *curr;

//     cli();

//     GETPRO(curr);

//     curr->state = RUNNABLE;

//     list_add_tail(&curr->run_node, &rq->head);
    
//     schedule();

//     sti();
// }



// /**
//  * @brief task stopped and yield the CPU
//  * 
//  * @param task 
//  */
// void sched_sleep(thread_t *task) {
//     uint32_t intr_flag;
//     cli_and_save(intr_flag); 

//     task->state = SLEEPING;
//     schedule();

//     restore_flags(intr_flag);
// }



// void sched_wakeup(thread_t *task) {
//     uint32_t flag;
//     cli_and_save(flag);

//     task->state = RUNNABLE;

//     /* add back to the front for better performance */
//     list_add_tail(&task->run_node, &rq->head);
    
//     schedule();
//     restore_flags(flag);
// }

// /**
//  * @brief scheduler service routine
//  * 
//  */
// void schedule(void) {
//     thread_t *prev, *next;
//     list_head *node;

//     /* get current thread struct */
//     GETPRO(prev);        
    
//     node = rq->head.next;

//     list_del(node);

//     next = list_entry(node, thread_t, run_node);

//     /* update states */
//     next->state = RUNNING;
    
//     if (next == prev) return;        

//     /* update current console's task */
//     if (prev->console_id == next->console_id)
//         current->task = next;

//     context_switch(prev, next);
// }


// /**
//  * @brief halts the central processing unit (CPU) until 
//  * the next external interqupt is fired.
//  * 
//  */
// void pause(void) {
//     /* avoid deadlock by ensuring that devices can interqupt */
//     sti();

//     /* Spin (nicely, so we don't chew up cycles) */
//     asm volatile (".1: hlt; jmp .1;");
// }
