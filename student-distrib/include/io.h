#ifndef _IO_H
#define _IO_H

#include <types.h>

#define VIDEO       0xB8000
#define NUM_COLS    80
#define NUM_ROWS    25
#define ATTRIB      0x7

extern int screen_x;
extern int screen_y;

void clear(void);
int32_t printf(int8_t *format, ...);
void putc(uint8_t c);
int32_t puts(int8_t *s);
void test_interrupts(void);



#endif /* _IO_T */
