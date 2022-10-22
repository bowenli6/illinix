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


extern const char scancodes[KEYBOARD_SIZE][2];

void keyboard_init();
void do_keyboard();


#endif /* _KEYBOARD_H */
