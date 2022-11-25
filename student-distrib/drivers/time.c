#include <drivers/time.h>
#include <boot/i8259.h>
#include <pro/process.h>
#include <pro/sched.h>
#include <lib.h>
#include <io.h>

uint32_t sys_ticks;  /* stores the number of elapsed ticks since the system was started (up to 50 days) */
timespec sys_time;   /* current time and date */

/**
 * @brief init PIT (Programmable Interval Timer)
 * PIT issues timer interrupts at a (roughly) 1000-HZ
 * frequency (that is, once every 1 ms)
 * 
 */
void PIT_init(void) {
    /* init sys_time */
    // TODO

    /* set up PIT*/
    outb_p(0x34, CMD_REG);                /* binary, mode 2, LSB/MSB, ch 0 */
    outb_p(LATCH & 0xff, TIMER_CHANNEL);  /* LSB */
    outb(LATCH >> 8, TIMER_CHANNEL);      /* MSB */
    sys_ticks = 0;
    enable_irq(TIMER_IRQ);
}

/**
 * @brief timer interrupt handler
 * 
 */
void do_timer(void) {
    uint32_t intr_flag;
    thread_t *current;

    sys_ticks++;
    runqueue->clock += TICKUNIT;
    send_eoi(TIMER_IRQ);      

    /* critical section begins. */
    cli_and_save(intr_flag);  

    // TODO update sys_time

    /* update vruntime of current task and schedule as needed */
    if (update_curr(&current->sched_info))
        schedule();

    /* critical section ends. */
    restore_flags(intr_flag);
}
