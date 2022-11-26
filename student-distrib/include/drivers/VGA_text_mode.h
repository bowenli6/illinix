#include <types.h>

#define SCREEN 0xB8000
#define WIDTH 80
#define HEIGHT 25
#define TERMINAL_MAX 3

#define VGA_GRAPH_CMD_port 0x01ce
#define VGA_GRAPH_DATA_port 0x01cf

#define VGA_TEXT_CMD_port 0x3d4
#define VGA_TEXT_DATA_port 0x3d5

#define VGA_HEIGHT_OFFSET 9

#define CURSOR_high 0x0e
#define CURSOR_low  0x0f

extern uint16_t VGA_witdh_resoltion;
extern uint16_t VGA_height_resolution;
extern uint16_t VGA_bit_per_pixel;
extern uint32_t VGA_enabled;

void VGA_init(uint16_t width, uint16_t height, uint16_t bitPixel);
void VGA_write(uint16_t REG_index, uint16_t data);
void VGA_switch_terminal(int32_t terminal_index);
void VGA_text_set_cursor_position(uint8_t x, uint8_t y);