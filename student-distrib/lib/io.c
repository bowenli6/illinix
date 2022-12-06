#include <io.h>
#include <lib.h>
#include <boot/syscall.h>
#include <pro/process.h>
#include <access.h>
#include <drivers/vga.h>
#include <drivers/terminal.h>


int screen_x = 0;
int screen_y = 0;


int32_t fputs(int32_t fd, const int8_t* s) {
    uint32_t size = strlen(s);
    return sys_write (fd, s, size);
}

/**
 * @brief Clears video memory 
 */
void clear(void) {
    thread_t *curr;
    terminal_t *terminal;

    if (!terminal_boot) {
        vga_clear(video_mem);
        screen_x = 0;
        screen_y = 0;
        return;
    }

    curr = current->task;
    terminal = curr->terminal;
    
    vga_clear(terminal->vidmem);
    terminal->screen_x = 0;
    terminal->screen_y = 0;
}



/**
 * @brief Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output. 
 */
int32_t printf(int8_t *format, ...) {

    /* Pointer to the format string */
    int8_t* buf = format;

    /* Stack pointer for the other parameters */
    int32_t* esp = (void *)&format;
    esp++;

    while (*buf != '\0') {
        switch (*buf) {
            case '%':
                {
                    int32_t alternate = 0;
                    buf++;

format_char_switch:
                    /* Conversion specifiers */
                    switch (*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            putc('%');
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                int8_t conv_buf[64];
                                if (alternate == 0) {
                                    itoa(*((uint32_t *)esp), conv_buf, 16);
                                    puts(conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    puts(&conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a number in signed int form */
                        case 'd':
                            {
                                int8_t conv_buf[36];
                                int32_t value = *((int32_t *)esp);
                                if(value < 0) {
                                    conv_buf[0] = '-';
                                    itoa(-value, &conv_buf[1], 10);
                                } else {
                                    itoa(value, conv_buf, 10);
                                }
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            putc((uint8_t) *((int32_t *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            puts(*((int8_t **)esp));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                putc(*buf);
                break;
        }
        buf++;
    }
    return (buf - format);
}

/** 
 * @brief Output a string to the console.
 * @param s : pointer to a string of characters.
 * @return Number of bytes written.
 */
int32_t puts(int8_t* s) {
    register int32_t index = 0;
    while (s[index] != '\0') {
        putc(s[index]);
        index++;
    }
    return index;
}

void _putc(uint8_t c, terminal_t* terminal) {
    int32_t x, y;
    uint8_t *vidmem;

    if (!terminal_boot) {
        x = screen_x;
        y = screen_y;
        vidmem = (uint8_t*)video_mem;
    } else {  
        x = terminal->screen_x;
        y = terminal->screen_y;
        vidmem = terminal->vidmem;
    }
     if(c == '\n' || c == '\r') {
        if (y + 1 == NUM_ROWS) 
            vga_scrolling(vidmem);
        else
           y++; 
        x = 0;
    } else {
        vga_write(vidmem, x, y, c);
        x++;
        x %= NUM_COLS;
        if (x == 0) {
            if (y + 1 != NUM_ROWS)
                y++;
            else
                vga_scrolling(vidmem);
        }
    }
    if (!terminal_boot) {
        screen_x = x;
        screen_y = y;
        vga_update_cursor(x, y);
    } else {
        terminal->screen_x = x;
        terminal->screen_y = y;
        if (terminal == current->task->terminal)
            vga_update_cursor(x, y);
    }
}

/**
 * @brief Output a character to the console .
 * @param c : character to print.
 */
void putc(uint8_t c) {
    thread_t *curr;
    terminal_t *terminal;
    if(terminal_boot) {
        GETPRO(curr);
        terminal = curr->terminal;
    }
    _putc(c, terminal);
}


/* void test_interrupts(void)
 * Inputs: void
 * Return Value: void
 * Function: increments video memory. To be used to test rtc */
void test_interrupts(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        video_mem[i << 1]++;
    }
}

/* A tmp function used to backspace one character. 
 * Will be deleted when VGA is supported.
 */
void back(terminal_t *terminal) {
    if (terminal->screen_x == 0 && terminal->screen_y == 0) 
        return;
    if (terminal->screen_x == 0) {
        terminal->screen_x = NUM_COLS - 1;
        --terminal->screen_y;
    } else {
        terminal->screen_x--;
    }
    vga_write(terminal->vidmem, terminal->screen_x, terminal->screen_y, ' ');
    if (terminal == current->task->terminal)
        vga_update_cursor(terminal->screen_x, terminal->screen_y);
}

void panic(int8_t *s)
{
    cli();
    puts(s);
    printf("\n");
    while(1);
}
