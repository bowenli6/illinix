#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include <types.h>

/* Reference: https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html */
#define KEYBOARD_IRQ    1                   /* IRQ port 1 of PIC. */
#define KEYBOARD_PORT   0x60                /* Keyboard port. */
#define KEYBOARD_SIZE   58                  /* The size of the scancodes buffer. */
#define SCANCODES_SIZE  128                 /* The total number of scannode. */
#define LSHIFT          0x2a                /* Left shift key. */
#define RSHIFT          0x36                /* Left shift key. */
#define BACKSPACE       0x0e                /* Backspace key. */
#define CAPSLOCK        0x3a                /* CapsLock key. */
#define CTRL            0x1d                /* Ctrl key. */
#define L               0x26                /* L key. */
#define D               0x20                /* D key. */
#define Z               0x2c                /* C key. */          


/* Keyboard scancodes
 * The data from a keyboard comes mainly in the form of scancodes, 
 * produced by key presses or used in the protocol with the computer.
 * Not every single ASCII character has a scancode. Only the first 58
 * do. The index 0 of the string is the lower case character, and the
 * index 1 of the string is the upper case character.  
 */
static const char scancodes[KEYBOARD_SIZE][2] = 
{                                                                                     /* Backspace*/
    {0, 0}, {27, 0}, "1!", "2@", "3#", "4$", "5%", "6^", "7&", "8*", "9(", "0)", "-_", "=+", {8, 8}, 
    {9, 9} /* Tab*/, "qQ", "wW", "eE", "rR", "tT", "yY", "uU", "iI", "oO", "pP", "[{", "]}", 
    {13, 13} /* Enter*/, {0, 0}, "aA", "sS", "dD", "fF", "gG", "hH", "jJ", "kK", "lL", ";:", {39, 34}, /* "' */
    "`~", {0, 0}, {92, '|'} /* \ */  , "zZ", "xX", "cC", "vV", "bB", "nN", "mM", ",<", ".>", "/?",
    {0, 0}, {0, 0}, {0, 0}, "  " /* space */
};


void keyboard_init();
void do_keyboard();
void key_press(uint32_t scancode);      /* Implmented by terminal.c */
void key_release(uint32_t scancode);    /* Implmented by terminal.c */


#endif /* _KEYBOARD_H */

