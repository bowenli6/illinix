#include <drivers/time.h>
#include <boot/i8259.h>
#include <lib.h>
#include <io.h>

/**
 * @brief init PIT (Programmable Interval Timer)
 * 
 */
void PIT_init(void) {
    outb(0x36, CMD_REG);                /* binary, mode 2, LSB/MSB, ch 0 */
    outb(LATCH & 0xff, TIMER_CHANNEL);  /* LSB */
    outb(LATCH >> 8, TIMER_CHANNEL);
    enable_irq(TIMER_IRQ);
}

/**
 * @brief timer interrupt handler
 * 
 */
void do_timer(void) {
    uint32_t intr_flag;
    send_eoi(TIMER_IRQ);                     /* Send End of interrupt to the PIC. */

    /* Critical section begins. */
    cli_and_save(intr_flag);  

    /* TODO */

    /* Critical section ends. */
    restore_flags(intr_flag);
}
