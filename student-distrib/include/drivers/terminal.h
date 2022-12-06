#ifndef _TERMAINL_H
#define _TERMAINL_H

#include <drivers/keyboard.h>

#define TERBUF_SIZE 128                 /* max buffer size */
#define VIDMEM_SIZE 4096                /* video memory size */

typedef struct {
    uint8_t capslock;                   /* 0 if capslock is not pressed, 1 otherwise. */
    uint8_t shift;                      /* 0 if Lshift and Rshift are not pressed, 1 otherwise. */
    uint8_t ctrl;                       /* 0 if Lctrl and Rctrl are not pressed, 1 otherwise. */
    uint8_t alt;                        /* 0 if Lalt and Ralt are not pressed, 1 otherwise. */
    uint8_t bufhd;                      /* The top position of the buffer. */
    uint8_t buftl;                      /* The bottom position of the buffer. */
    uint8_t size;                       /* The current size of the buffer. */
    uint8_t *buffer;                    /* Line buffer input. */
    uint8_t exit;                       /* A flag for stdin, 1 if \n is detected. */
    uint8_t screen_x;                   /* cursor column index */
    uint8_t screen_y;                   /* cursor row index */
    char *vidmem;                    /* 4KB video memory for this terminal */ 
    char *saved_vidmem;              /* saved video memory address for backing up */
} terminal_t;

extern int8_t terminal_boot;


void key_press(uint32_t scancode, terminal_t *terminal);
void key_release(uint32_t scancode, terminal_t *terminal);
terminal_t *terminal_create(void);
void terminal_free(terminal_t *terminal);
int32_t terminal_open(const int8_t *fname);
int32_t terminal_close(int32_t fd);
int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const void *buf, int32_t nbytes);


#endif /*_TERMAINL_H */
