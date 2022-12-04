#include <drivers/time.h>
#include <boot/i8259.h>
#include <pro/process.h>
#include <pro/sched.h>
// #include <pro/cfs.h>
#include <lib.h>
#include <io.h>

volatile uint32_t sys_ticks;  /* stores the number of elapsed ticks since the system was started (up to 50 days) */
timespec sys_clock;           /* current time and date */

/**
 * @brief init PIT (Programmable Interval Timer)
 * PIT issues timer interrupts at a (roughly) 1000-HZ
 * frequency (that is, once every 1 ms)
 * 
 */
void pit_init(void) {
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

    /* update system clock */
    // sys_ticks++;
    
    /* update scheduler clock 
     * NOTE: the waying of doing this might be updated 
     * when high precision system clock is enabled 
     */
    // TODO (cfs)

    send_eoi(TIMER_IRQ);       

    /* tick ... */
    sched_tick();

    // GETPRO(current);

    // /* update vruntime of current task and reschedule when needed */
    // task_tick(current);

    // if (current->flag == NEED_RESCHED) {
    //     restore_flags(intr_flag);
    //     schedule();
    //     return;
    // }

}
