#include <drivers/time.h>
#include <pro/process.h>
#include <pro/sched.h>


/**
 * @brief timer interrupt handler
 * 
 */
void do_timer(void) {
    cli();

    /* check if the current thread has remaining time slice */
    if ((--CURRENT->time_slice) > 0) {
        sti();
        return;
    }

    /* invoke process 0 when the time slice is zero */
    context_switch(CURRENT->context, sched->context);
}