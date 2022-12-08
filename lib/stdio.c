#include "../include/stdio.h"
#include "../include/stdlib.h"
#include "../include/unistd.h"


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

}


