#include "keyboard.h"
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
 * @brief Interrupt handler for the keyboard device.
 */
void do_keyboard() {
    cli();
    uint8_t scancode = inb(KEYBOARD_PORT);
    if (scancode < SCANCODES_SIZE)
        keyboard_print(scancode);
    send_eoi(KEYBOARD_IRQ);
    sti();
}


/**
 * @brief Print the character that specified by the given keyboard scancode.
 * 
 * @scancode: The keyboard scancode of the input character.
 */
static void keyboard_print(uint8_t scancode) {
    uint8_t character = scancodes[scancode][0];
    if (character)
        putc(character);
}

