#ifndef _VGA_H_
#define _VGA_H_

#include <types.h>
#include <drivers/terminal.h>

#define SCANSTART       0
#define SCANEND         15

#define VIDEO       0xB8000
#define ATTRIB      0x7

#define VGA_WIDTH   80
#define VGA_HEIGHT  25

extern char *video_mem;

void vga_init(void);
void vga_write(char *vidmem, uint8_t x, uint8_t y, int8_t c);
void vga_clear(char *vidmem);
void vga_scrolling(char *vidmem);
void vga_enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void vga_disable_cursor(void);
void vga_update_cursor(uint8_t x, uint8_t y);

#endif /* _VGA_H_ */
