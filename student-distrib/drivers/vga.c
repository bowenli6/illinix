#include <drivers/vga.h>
#include <lib.h>


/* video memory pointer */
char *video_mem = (char *)VIDEO;


/**
 * @brief init the vga driver
 * 
 */
void vga_init(void) {
    vga_enable_cursor(SCANSTART, SCANEND);
}


/**
 * @brief write a char to the screen
 * 
 * @param x : col position
 * @param y : row position
 */
void vga_write(char *vidmem, uint8_t x, uint8_t y, int8_t c) {
	*(uint8_t *)(vidmem + ((VGA_WIDTH * y + x) << 1)) = c;
    *(uint8_t *)(vidmem + ((VGA_WIDTH * y + x) << 1) + 1) = ATTRIB;
}


/**
 * @brief clear the screen
 * 
 */
void vga_clear(char *vidmem) {
	int32_t i;
    for (i = 0; i < VGA_HEIGHT * VGA_WIDTH; i++) {
        *(uint8_t *)(vidmem + (i << 1)) = ' ';
        *(uint8_t *)(vidmem + (i << 1) + 1) = ATTRIB;
    }
}



/**
 * @brief vertical scrolling down the screen
 * 
 */
void vga_scrolling(char *vidmem) {
	int32_t i;

    for (i = 0; i < VGA_WIDTH * (VGA_HEIGHT - 1); ++i) {
        *(uint8_t *)(vidmem + (i << 1)) = *(uint8_t *)(vidmem + ((i + VGA_WIDTH) << 1));
        *(uint8_t *)(vidmem + (i << 1) + 1) = ATTRIB;
    }

    for (i = VGA_WIDTH * (VGA_HEIGHT - 1); i < VGA_WIDTH * VGA_HEIGHT; ++i) {
        *(uint8_t *)(vidmem + (i << 1)) = ' ';
        *(uint8_t *)(vidmem + (i << 1) + 1) = ATTRIB;
    }
}

/**
 * @brief enable text-mode cursor
 * 
 *  Cursor Start Register (Index 0Ah)
 *  -------------------------------------------------
 *  |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 *  -------------------------------------------------
 *   |     |     | CD  |     Cursor Scan Line Start  |
 *  -------------------------------------------------
 *  CD -- Cursor Disable
 *
 * This field controls whether or not the text-mode cursor is displayed. Values are:
 * 
 * 0 -- Cursor Enabled
 * 
 * 1 -- Cursor Disabled
 * 
 * @param cursor_start : start scanline
 * @param cursor_end : end scanline
 */
void vga_enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
	outb(0xA, 0x3D4);
	outb((inb(0x3D5) & 0xC0) | cursor_start, 0x3D5);
 
	outb(0x0B, 0x3D4);
	outb((inb(0x3D5) & 0xE0) | cursor_end, 0x3D5);
}


/**
 * @brief disable text-mode cursor
 * 
 */
void vga_disable_cursor(void) {
	outb(0x0A, 0x3D4);
	outb(0x20, 0x3D5);
}



/**
 * @brief update cursor to row y and col y
 * 
 * @param x : col position
 * @param y : row position
 */
void vga_update_cursor(uint8_t x, uint8_t y) {
	uint16_t pos = y * VGA_WIDTH + x;
 
	outb(0x0F, 0x3D4);
	outb((uint8_t) (pos & 0xFF), 0x3D5);
	outb(0x0E, 0x3D4);
	outb((uint8_t) ((pos >> 8) & 0xFF), 0x3D5);
}

