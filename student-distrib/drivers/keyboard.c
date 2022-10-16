#include "keyboard.h"
#include "../include/i8259.h"

/* Local helper functions. */

static void keyboard_print(uint8_t scancode);


/**
 * keyboard_init
 * 
 * Initialize the keyboard and enable the interrput.
 */
void keyboard_init(void) {
    enable_irq(KEYBOARD_IRQ);
}

/**
 * keyboard_handler
 * 
 * Interrupt handler for the keyboard device.
 */
void keyboard_handler(void) {
    unsigned long interrupt_flag;
    cli_and_save(interrupt_flag);
    uint8_t scancode = inb(KEYBOARD_PORT);
    keyboard_print(scancode);
    send_eoi(KEYBOARD_IRQ);
    restore_flags(interrupt_flag);
}

/**
 * keyboard_print
 * @scancode: The keyboard scancode of the input character.
 * 
 * Print the character that specified by the given keyboard scancode.
 */
static void keyboard_print(uint8_t scancode) {
    uint8_t character = scancodes[scancode][0];
    if (character)
        printf("%c", character);
}

