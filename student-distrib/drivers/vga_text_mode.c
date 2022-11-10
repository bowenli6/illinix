#include "vga_text_mode.h"
#include ../include/"lib.h"
#include <stdint.h>



/** void vga_text_set_cursor_pos(uint8_t x, uint8_t y)
 * @output: the cursor on VGA text mode changes to (x, y)
 * @description: set the position of the blinking cursor.
 */
void vga_text_set_cursor_pos(uint8_t x, uint8_t y) {
    // Calculate the character position on VGA plane, row major order
    uint16_t char_pos = (y * SCREEN_WIDTH + x) % (SCREEN_WIDTH * SCREEN_HEIGHT);
    // Replace the value of VGA Cursor Location High Register with high 8 bits
    // of the calculated position
    outb(VGA_REG_CURSOR_LOCATION_HIGH, VGA_PORT_INDEX);
    outb((uint8_t) (char_pos >> 8), VGA_PORT_DATA);
    // Replace the value of VGA Cursor Location Low Register with low 8 bits
    // of the calculated position
    outb(VGA_REG_CURSOR_LOCATION_LOW, VGA_PORT_INDEX);
    outb((uint8_t) char_pos, VGA_PORT_DATA)
}