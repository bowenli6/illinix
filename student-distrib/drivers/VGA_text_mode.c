#include "../include/drivers/VGA_text_mode.h"
#include "../include/lib.h"
#include "../include/pro/process.h"


// initialize VGA status
uint16_t VGA_witdh_resoltion = 0;
uint16_t VGA_height_resolution = 0;
uint16_t VGA_bit_per_pixel = 0;
uint32_t VGA_enabled = 0;



void VGA_init(uint16_t width, uint16_t height, uint16_t bitPixel){

    /* there should have some sanity check for version and VGA address*/

    VGA_witdh_resoltion = width;
    VGA_height_resolution = height;
    VGA_bit_per_pixel = bitPixel;

    VGA_write(0xe0, 4);     // VGA_enable
    VGA_write(width, 1);    // VGA width resolution
    VGA_write(height, 2);   // VGA height resolution
    VGA_write(bitPixel, 3); // VGA bit per pixel
    VGA_write(width, 6);    // vertical width?
    VGA_write(0, 8);        // x offset
    VGA_write(0, 9);        // y offset
    VGA_write(0x61, 4);     // VGA enable clear

    VGA_enabled = 1;

}


/**
 * @brief write the data to the VGA register specified by REG_index
*/
void VGA_write(uint16_t data, uint16_t REG_index){
    outw(REG_index, VGA_GRAPH_CMD_port);
    outw(data, VGA_GRAPH_DATA_port);
}

/**
 * @brief Terminals stored continuously in linear buffer:
 * by changing of Y offset, we can switch between these terminals.
 * 
 */
// terminal index will be implemented
void VGA_switch_terminal(int32_t terminal_index){
    if(!VGA_enabled || terminal_index >= TERMINAL_MAX) {
        return;
    }
    VGA_write((terminal_index * VGA_height_resolution), VGA_HEIGHT_OFFSET);
}


void vga_text_set_color(uint8_t x, uint8_t y, uint8_t foreground, uint8_t background) {
    /* read the color from input */
    uint8_t color = (background & 0xF) << 4 | (foreground & 0xF);
    /* set the char at the specific location to color */
    *(uint8_t*) (SCREEN + (y * WIDTH + x) * 2 + 1) = color;
}


/** 
 *  @brief set the position of the cursor based on x,y given
 */
void VGA_text_set_cursor_position(uint8_t x, uint8_t y){
    // TODO: compare the terminal id and display id !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11

    /* calculate the position of cursor*/
    uint16_t cursor_pos = (y * WIDTH + x) % (WIDTH * HEIGHT);
    
    uint8_t cursor_hipos = (uint8_t)(cursor_pos>>8);
    uint8_t cursor_lowpos = (uint8_t)(cursor_pos);
    /* update the cursor*/
    outb(CURSOR_high, VGA_TEXT_CMD_port);
    /* replace VGA cursor high bit to the upper 8 bits */
    outb(cursor_hipos ,VGA_TEXT_DATA_port);
    /* replace VGA cursor low bit to the lower 8 bits */
    outb(CURSOR_low, VGA_TEXT_CMD_port);
    /* replace VGA cursor high bit to the upper 8 bits */
    outb(cursor_lowpos ,VGA_TEXT_DATA_port);

}









