#include <drivers/keyboard.h>
#include <boot/i8259.h>
#include <lib.h>


/**
 * @brief Initialize the keyboard and enable the interrput.
 */
void keyboard_init() {
    enable_irq(KEYBOARD_IRQ);
}


/**
 * @brief Interrupt handler for the keyboard device.
 */
void do_keyboard() {
    uint32_t scancode = inb(KEYBOARD_PORT);      /* Read one byte from stdin. */
    send_eoi(KEYBOARD_IRQ);                     /* Send End of interrupt to the PIC. */

    /* critical section begins. */
    cli();  

    if (scancode < SCANCODES_SIZE)              /* key press (make) */
        key_press(scancode);
    else                                        /* key release (break) */
        key_release(scancode - SCANCODES_SIZE);

    /* critical section ends. */
    sti();
}
