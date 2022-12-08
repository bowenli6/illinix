#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


/**
 * @brief Write output to stdout.
 * 
 * @param format : format of the string
 * @param ... : arguments matched to the format
 * @return int : Upon successful return, these functions return 
 * the number of characters printed (excluding the null byte used 
 * to end output to strings).
 */
int printf(const char *format, ...) {

    /* Pointer to the format string */
    char *buf = (char *)format;

    /* Stack pointer for the other parameters */
    int *esp = (void *)&format;
    esp++;

    while (*buf != '\0') {
        switch (*buf) {
            case '%':
                {
                    int alternate = 0;
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
                                char conv_buf[64];
                                if (alternate == 0) {
                                    itoa(*((unsigned int *)esp), conv_buf, 16);
                                    puts(conv_buf);
                                } else {
                                    int starting_index;
                                    int i;
                                    itoa(*((unsigned int *)esp), &conv_buf[8], 16);
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
                                char conv_buf[36];
                                itoa(*((unsigned int *)esp), conv_buf, 10);
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a number in signed int form */
                        case 'd':
                            {
                                char conv_buf[36];
                                int value = *((int *)esp);
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
                            putc((unsigned char) *((int *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            puts(*((char **)esp));
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
 * @brief writes the string s and a trailing newline to stdout.
 * 
 * @param s : string s
 * @return int : return a nonnegative number on success, or EOF on error.
 */
int puts(char *s) {
    register int index = 0;
    while (s[index] != '\0') {
        putc(s[index]);
        index++;
    }
    return index;
}


/**
 * @brief Writes the character c, cast to an unsigned char, to stream.
 * 
 * @param c : char to write
 */
void putc(unsigned char c) {
    char s[2];
    s[0] = (char) c;
    write(1, s, 1);
}


/**
 * @brief Write to the character string str.
 * 
 * @param str 
 * @param format 
 * @param ... 
 * @return int 
 */
int sprintf(char *str, const char *format, ...) {
    return 0;
}


/**
 * @brief write output to the given output stream;
 *
 * @param fd : file descriptor
 * @param format : format of the string
 * @param ... : arguments matched to the format
 * @return int : Upon successful return, these functions return 
 * the number of characters printed (excluding the null byte used 
 * to end output to strings). 
 */
int fprintf(int fd, const char *format, ...) {
    return 0;
}


/**
 * @brief Reads input from the standard input stream stdin
 * 
 * @param format format of the string
 * @param ... : arguments matched to the format
 * @return int : On success,return the number of input items success fully
 *  matched and assigned; this can be fewer than provided for, or even zero, 
 * in the event of an early matching failure.
 */
int scanf(const char *format, ...) {
    return 0;
}



/**
 * @brief writes the string s to stream, without its terminating null
 * byte ('\0').
 * 
 * @param fd : file descriptor
 * @param s : string 
 * @return int : number of bytes write
 */
int fputs(int fd, const char *s) {
    return write(fd, (void*)s, strlen(s));
}


/**
 * @brief reads in at most one less than size characters from stream and
 * stores them into the buffer pointed to by s. Reading stops after an EOF 
 * or a newline. If a newline is read, it is stored into the buffer. 
 * A terminating null byte ('\0') is stored after the last character in 
 * the buffer.
 * 
 * @param s : store buffer into s
 * @param size : numebr of bytes to read
 * @param stream : input stream
 * @return char* : returns s on success, and NULL on error or when end 
 * of file occurs while no characters have been read.
 */
char *fgets(char *s, int size, int stream) {
    /* EOP unimplemented due to lack of signal module */

    if (read(stream, (void *)s, size) < 0) {
        return NULL;
    }

    return s;
}


