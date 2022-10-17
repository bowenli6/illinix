#include "rtc.h"
#include "../lib/lib.h"
#include "../include/i8259.h"

/* Claimed as volatile to let it change base on interrupts. */
volatile int global_interrupt_flag;

/* local helper functions*/
void set_RTC_freq(int32_t frequency);
char log2_of(int32_t frequency);


/**
 * @brief Initialize RTC and enable RTC interrupt.
 * 
 */
void RTC_init() {
    /* Reference from https://wiki.osdev.org/RTC#Turning_on_IRQ_8 and Linux source code.
     * might neeed to turn off interrupts if multiprocessor involved
     */
    outb(RTC_B_reg, RTC_CMD_port);	    /* Select register B, and disable NMI. */
    char prev = inb(RTC_DATA_port);	    /* Read the current value of register B */
    outb(RTC_B_reg, RTC_CMD_port);	    /* Set the index again (a read will reset the index to register D) */
    outb(prev | 0x40, RTC_DATA_port);   /* Write the previous value ORed with 0x40. This turns on bit 6 of register B */
    enable_irq(RTC_IRQ);
    set_RTC_freq(RTC_MAX_freq);
    global_interrupt_flag = 0;
}

/**
 * @brief Read data from register C and handle it
 * 
 */
void do_RTC() {
    uint32_t interrupt_flag;
    global_interrupt_flag = 1;
    cli_and_save(interrupt_flag);       /* Disable interrupts and store flags into local variable. */
    test_interrupts();                  /* Execute when an RTC interrupt occurs. */
    set_RTC_freq(RTC_MIN_freq);
    send_eoi(RTC_IRQ);
    restore_flags(interrupt_flag);

    outb(RTC_C_reg, RTC_CMD_port);     /* read from register C and ensure all interrupts are properly generated */

    inb(RTC_DATA_port);                /* discard the value for now. */
}


/**
 * @brief local helper function that set the RTC frequency to the given value
*/
void set_RTC_freq(int32_t frequency) {
    // frequency =  32768 >> (rate-1)
    char rate = 15 - log2_of(frequency) + 1;
    rate &= rate_mask;

    if (rate < 3) {     // the minimum rate is 3
        return;
    }
    uint32_t interrupt_flag;
    cli_and_save(interrupt_flag);       /* Disable interrupts and store flags into local variable. */
    outb(RTC_A_reg, RTC_CMD_port);
	char prev = inb(RTC_DATA_port);
	outb(RTC_A_reg, RTC_CMD_port);
	outb((prev & prev_mask) | rate, RTC_DATA_port);
    restore_flags(interrupt_flag);
}


char log2_of(int32_t frequency) {
    char count = 0;
    while(frequency != 1){
        frequency = frequency >> 1;
        count++;
    }
    return count;
}


