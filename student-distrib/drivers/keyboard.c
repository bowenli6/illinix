#include <drivers/keyboard.h>
#include <drivers/terminal.h>
#include <pro/process.h>
#include <boot/i8259.h>
#include <lib.h>
#include <io.h>


/* Keyboard scancodes
 * The data from a keyboard comes mainly in the form of scancodes, 
 * produced by key presses or used in the protocol with the computer.
 * Not every single ASCII character has a scancode. Only the first 58
 * do. The index 0 of the string is the lower case character, and the
 * index 1 of the string is the upper case character.  
 */
const char scancodes[KEYBOARD_SIZE][2] = 
{                                                                                     /* Backspace*/
    {0, 0}, {27, 0}, "1!", "2@", "3#", "4$", "5%", "6^", "7&", "8*", "9(", "0)", "-_", "=+", {8, 8}, 
    {0, 0} /* Tab*/, "qQ", "wW", "eE", "rR", "tT", "yY", "uU", "iI", "oO", "pP", "[{", "]}", 
    {13, 13} /* Enter*/, {0, 0}, "aA", "sS", "dD", "fF", "gG", "hH", "jJ", "kK", "lL", ";:", {39, 34}, /* "' */
    "`~", {0, 0}, {92, '|'} /* \ */  , "zZ", "xX", "cC", "vV", "bB", "nN", "mM", ",<", ".>", "/?",
    {0, 0}, {0, 0}, {0, 0}, "  " /* space */
};




/**
 * @brief Initialize the keyboard and enable the interrput.
 */
void keyboard_init(void) {
    enable_irq(KEYBOARD_IRQ);
}


/**
 * @brief Interrupt handler for the keyboard device.
 */
void do_keyboard(void) {
    terminal_t *terminal;
    uint32_t scancode;

    /* Critical section begins. */
    cli();

    scancode = inb(KEYBOARD_PORT);              /* Read one byte from stdin. */

    send_eoi(KEYBOARD_IRQ);                     /* Send End of interrupt to the PIC. */

    terminal = current->task->terminal;

    if (scancode < SCANCODES_SIZE)              /* key press (make) */
        key_press(scancode, terminal);
    else                                        /* key release (break) */
        key_release(scancode - SCANCODES_SIZE, terminal);

    /* Critical section ends. */
    sti();
}
