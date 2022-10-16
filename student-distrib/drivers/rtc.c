
/*RTC handler*/

#include "rtc.h"
#include "../lib/lib.h"
#include "../include/i8259.h"

/* Claimed as volatile to let it change base on interrupts. */
volatile int global_interrupt_flag;

/**
 * RTC_init
 * description: initialize RTC and enable RTC interrupt
 * input: void
 * output: void
 */

void RTC_init(void) {
    /* Reference from https://wiki.osdev.org/RTC#Turning_on_IRQ_8 and Linux source code.
     * might neeed to turn off interrupts if multiprocessor involved
     */
    outb(RTC_B_reg, RTC_CMD_port);	    /* Select register B, and disable NMI. */
    char prev = inb(RTC_DATA_port);	    /* Read the current value of register B */
    outb(RTC_B_reg, RTC_CMD_port);	    /* Set the index again (a read will reset the index to register D) */
    outb(prev | 0x40, RTC_DATA_port);   /* Write the previous value ORed with 0x40. This turns on bit 6 of register B */
    enable_irq(IRQ_8);
}

/**
 * RTC_interrupt
 * description: Read data from register C and handle it
 * input: void
 * output: void
 */

void RTC_handler(void) {
    unsigned long interrupt_flag;
    global_interrupt_flag = 1;
    cli_and_save(interrupt_flag);       /* Disable interrupts and store flags into local variable. */
    test_interrupts();                  /* Execute when an RTC interrupt occurs. */
    send_eoi(IRQ_8);
    restore_flags(interrupt_flag);
    
    // outb(RTC_C_reg, RTC_CMD_port);     // read from register C and ensure all interrupts are properly generated

    // inb(RTC_DATA_port);                // discard the value
}

