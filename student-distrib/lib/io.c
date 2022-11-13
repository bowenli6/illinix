#include <io.h>
#include <lib.h>
#include <boot/syscall.h>
#include <drivers/terminal.h>

int screen_x = 0;
int screen_y = 0;
static char* video_mem = (char *)VIDEO;


static void overflow();


int32_t fputs(int32_t fd, const int8_t* s) {
    uint32_t size = strlen(s);
    return sys_write (fd, s, size);
}

/**
 * @brief Clears video memory 
 */
void clear(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;
    }
    screen_x = 0;
    screen_y = 0;
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

/**
 * @brief Output a character to the console .
 * @param c : character to print.
 */
void putc(uint8_t c) {
    if(c == '\n' || c == '\r') {
        if (screen_y + 1 == NUM_ROWS) 
            overflow();
        else
           screen_y++; 
        screen_x = 0;
    } else {
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1)) = c;
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1) + 1) = ATTRIB;
        screen_x++;
        screen_x %= NUM_COLS;
        if (screen_x == 0) {
            if (screen_y + 1 != NUM_ROWS)
                screen_y++;
            else
                overflow();
        }
    }
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
void back() {
    if (screen_x == 0 && screen_y == 0) 
        return;
    if (screen_x == 0) {
        screen_x = NUM_COLS - 1;
        --screen_y;
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1)) = ' ';
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1) + 1) = ATTRIB;
    } else {
        screen_x--;
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1)) = ' ';
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1) + 1) = ATTRIB;
    }
}

/* A tmp function used to vertical scrolling. 
 * Will be deleted when VGA is supported.
 */
static void overflow() {
    int32_t i;
    for (i = 0; i < NUM_COLS * (NUM_ROWS - 1); ++i) {
        *(uint8_t *)(video_mem + (i << 1)) = *(uint8_t *)(video_mem + ((i + NUM_COLS) << 1));
        *(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;
    }

    for (i = NUM_COLS * (NUM_ROWS - 1); i < NUM_COLS * NUM_ROWS; ++i) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;
    }
}

