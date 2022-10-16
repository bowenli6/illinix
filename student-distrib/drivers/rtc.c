
/*RTC handler*/

#include "rtc.h"
#include "lib/lib.h"
#include "include/i8259.h"

// claimed as volatile to let it change base on interrupts
volatile int global_interrupt_flag;
/**
 * RTC_init
 * description: initialize RTC and enable RTC interrupt
 * input: void
 * output: void
 */

void RTC_init(void) {
    // Reference from https://wiki.osdev.org/RTC#Turning_on_IRQ_8 and Linux source code
    // might neeed to turn off interrupts if multiprocessor involved
    outb(RTC_B_reg, RTC_CMD_port);	    // select register B, and disable NMI
    char prev = inb(RTC_DATA_port);	    // read the current value of register B
    outb(RTC_B_reg, RTC_CMD_port);	    // set the index again (a read will reset the index to register D)
    outb(prev | 0x40, RTC_DATA_port);   // write the previous value ORed with 0x40. This turns on bit 6 of register B

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
    cli_and_save(interrupt_flag);      // disable interrupts and store flags into local variable

    outb(RTC_C_reg, RTC_CMD_port);     // read from register C and ensure all interrupts are properly generated

    inb(RTC_DATA_port);                // discard the value

    restore_flags(interrupt_flag);

    send_eoi(IRQ_8);
}

