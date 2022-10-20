#ifndef _TERMAINL_H
#define _TERMAINL_H

#include <drivers/keyboard.h>


typedef struct {
    uint8_t capslock;                   /* 0 if capslock is not pressed, 1 otherwise. */
    uint8_t shift;                      /* 0 if Lshift and Rshift are not pressed, 1 otherwise. */
    uint8_t ctrl;                       /* 0 if Lctrl and Rctrl are not pressed, 1 otherwise. */
    char buffer[KEYBOARD_SIZE];         /* Line buffer input. */
} terminal_t;


terminal_t terminal;

void terminal_init();
int terminal_open();
int terminal_close();
int terminal_read();
int terminal_write();

#endif /*_TERMAINL_H */
