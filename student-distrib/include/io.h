#ifndef _IO_H
#define _IO_H

#include <types.h>
#include <drivers/terminal.h>

#define VIDEO       0xB8000
#define NUM_COLS    80
#define NUM_ROWS    25
#define ATTRIB      0x7


void clear(void);
int32_t printf(int8_t *format, ...);
void putc(uint8_t c);
int32_t puts(int8_t *s);
void test_interrupts(void);
void back(terminal_t *terminal);
int32_t fputs(int32_t fd, const int8_t* s);
void _putc(uint8_t c, terminal_t* terminal);

void panic(int8_t* s);
#endif /* _IO_T */
