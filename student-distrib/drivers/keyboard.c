#include "keyboard.h"
#include "../include/interrupt.h"
#include "../include/i8259.h"

/* Local helper functions. */

static void keyboard_print(uint8_t scancode);


/**
 * @brief Initialize the keyboard and enable the interrput.
 */
void keyboard_init() {
    enable_irq(KEYBOARD_IRQ);
}

/**
 * keyboard_handler
 * 
 * Interrupt handler for the keyboard device.
 */
void keyboard_handler() {
    printf("keyboard interrupt detected!\n");
    cli();
    uint8_t scancode = inb(KEYBOARD_PORT);
    keyboard_print(scancode);
    send_eoi(KEYBOARD_IRQ);
    sti();
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
        putc(character);
}

