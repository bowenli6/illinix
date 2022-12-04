#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include <types.h>

/* Reference: https://wwwwintuenl/~aeb/linux/kbd/scancodes-1html */
#define KEYBOARD_IRQ    1                   /* IRQ port 1 of PIC */
#define KEYBOARD_PORT   0x60                /* Keyboard port */
#define KEYBOARD_SIZE   58                  /* The size of the scancodes buffer */
#define SCANCODES_SIZE  128                 /* The total number of scannode */
#define LSHIFT          0x2a                /* Left shift key */
#define RSHIFT          0x36                /* Left shift key */
#define BACKSPACE       0x0e                /* Backspace key */
#define CAPSLOCK        0x3a                /* CapsLock key */
#define SPACE           0x39                /* Space key */
#define CTRL            0x1d                /* Ctrl key */
#define LALT            0x38                /* Left alt key */
#define RALT            0xe0                /* Right alt key */
#define ENTER           0x1c                /* Enter key */
#define TAB             0x0f                /* Tab key */
#define TAB_SPACE       4                   /* Tab spaces */
#define F1              0x1E                /* F1 key */
#define F2              0x1F                /* F2 key */
#define F3              0x20                /* F3 key */
#define L               0x26                /* L key */
#define D               0x20                /* D key */
#define Z               0x2c                /* C key */     


extern const char scancodes[KEYBOARD_SIZE][2];

void keyboard_init(void);
void do_keyboard(void);


#endif /* _KEYBOARD_H */

