#include <drivers/rtc.h>
#include <boot/i8259.h>
#include <vfs/file.h>
#include <vfs/vfs.h>
#include <pro/process.h>
#include <drivers/fs.h>
#include <access.h>
#include <lib.h>
#include <io.h>


/* Claimed as volatile to let it change base on interrupts. */
volatile int global_interrupt_flag;

/* local helper functions*/
static void set_rtc_freq(int32_t frequency);
static char log2_of(int32_t frequency);

/* RTC operation. */
static file_op rtc_op = {
    .open = rtc_open,
    .close = rtc_close,
    .read = rtc_read,
    .write = rtc_write
};

/**
 * @brief Initialize RTC and enable RTC interrupt.
 * 
 */
void rtc_init() {
    /* Reference from https://wiki.osdev.org/RTC#Turning_on_IRQ_8 and Linux source code.
     * might neeed to turn off interrupts if multiprocessor involved
     */
    outb(RTC_B_reg, RTC_CMD_port);	    /* Select register B, and disable NMI. */
    char prev = inb(RTC_DATA_port);	    /* Read the current value of register B */
    outb(RTC_B_reg, RTC_CMD_port);	    /* Set the index again (a read will reset the index to register D) */
    outb(prev | 0x40, RTC_DATA_port);   /* Write the previous value ORed with 0x40. This turns on bit 6 of register B */
    enable_irq(RTC_IRQ);
    set_rtc_freq(RTC_MAX_freq);
    global_interrupt_flag = 0;
}

/**
 * @brief Read data from register C and handle it
 * 
 */
void do_rtc() {
    cli();
    global_interrupt_flag = 1;
    send_eoi(RTC_IRQ);

    outb(RTC_C_reg, RTC_CMD_port);     /* read from register C and ensure all interrupts are properly generated */

    inb(RTC_DATA_port);                /* discard the value for now. */
    sti();
}

/**
 * @brief Open the RTC.
 * 
 * @param filename : A filename.
 * @return int32_t : 0 on success, -1 otherwise.
 */
int32_t rtc_open(const int8_t* filename) {
    thread_t *curr;
    GETPRO(curr);
    /* initialize the RTC and set initial frequency to 2 as instructed */
    rtc_init();
    set_rtc_freq(2);
    return __open(2, filename, RTC, &rtc_op, curr);
}

/**
 * @brief Close RTC device
 * 
 * @param fd : The file descriptor
 * @return int32_t : 0 on success, -1 otherwise.
*/
int32_t rtc_close(int32_t fd) {
    return file_close(fd);
}

/**
 * @brief Wait for interrupt on RTC
 * 
 * @param fd : The file descriptor.
 * @param buffer : address of the target frequency.
 * @param nbytes : number of bytes.
 * 
 * @return int32_t : 0 on success, -1 otherwise.
 * 
*/
int32_t rtc_read(int32_t fd, void* buffer, int32_t nbytes) {
    /* wait on interrupt generation */
    while(!global_interrupt_flag);
    global_interrupt_flag = 0;
    return 0;
}


/**
 * @brief Set the RTC frequency based on buffer
 * 
 * @param fd : The file descriptor.
 * @param buffer : address of the target frequency.
 * @param nbytes : number of bytes.
 * 
 * @return int32_t : 0 on success, -1 otherwise.
 * 
*/
int32_t rtc_write(int32_t fd, const void* buffer, int32_t nbytes){
    /* Sanity check on new frequency */
    if (buffer == NULL || nbytes != sizeof(int32_t)) {
        return -1;
    }
    int32_t new_freq = *(int32_t*) buffer;
    /* check if the new frequency is out of bound*/
    if (new_freq > RTC_MAX_freq || new_freq < RTC_MIN_freq) {
        return -1;                      
    }
    /* check if the new frequency is power of 2*/
    if (new_freq & 0x1) {
        return -1;
    }

	set_rtc_freq(new_freq);
    return 0;
}



/**
 * @brief Local helper function that set the RTC frequency to the given value.
*/
static void set_rtc_freq(int32_t frequency) {
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


static char log2_of(int32_t frequency) {
    char count = 0;
    while(frequency != 1){
        frequency = frequency >> 1;
        count++;
    }
    return count;
}









