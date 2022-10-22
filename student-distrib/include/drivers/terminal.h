#ifndef _TERMAINL_H
#define _TERMAINL_H

#include <drivers/keyboard.h>

#define TERBUF_SIZE 128

typedef struct {
    uint8_t capslock;                   /* 0 if capslock is not pressed, 1 otherwise. */
    uint8_t shift;                      /* 0 if Lshift and Rshift are not pressed, 1 otherwise. */
    uint8_t ctrl;                       /* 0 if Lctrl and Rctrl are not pressed, 1 otherwise. */
    uint8_t bufhd;                      /* The top position of the buffer. */
    uint8_t buftl;                      /* The bottom position of the buffer. */
    uint8_t size;                       /* The current size of the buffer. */
    uint8_t buffer[TERBUF_SIZE];        /* Line buffer input. */
    uint8_t exit;                       /* A flag for stdin, 1 if \n is detected. */
} terminal_t;


extern terminal_t terminal;

void key_press(uint32_t scancode);
void key_release(uint32_t scancode);
int32_t terminal_open(const int8_t *fname);
int32_t terminal_close(int32_t fd);
int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const void *buf, int32_t nbytes);


#endif /*_TERMAINL_H */
