/**
 * @file string.c
 * @brief C Standard Library
 * @version 0.1
 * @date 2022-11-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include <string.h>
#include <stdlib.h>

/**
 * @brief calculates the length of the string pointed to by
 * s, excluding the terminating null byte ('\0').
 * 
 * @param s : input string
 * @return size_t : number of bytes in the string pointed
 * to by s.
 */
size_t strlen(const char *s) {
    register size_t len = 0;
    while (s[len]) ++len;
    return len;
}



/**
 * @brief compares the two strings s1 and s2.  
 * 
 * @param s1 : first source string
 * @param s2 : second source string
 * @return int : result of the comparison
 *               0, if the s1 and s2 are equal
 *               a negative value if s1 is less than s2
 *               a positive value if s1 is greater than s2
 */
int strcmp(const char *s1, const char *s2) {
    int i;
    for (i = 0; s1[i] && s2[i]; ++i) {
        if (s1[i] != s2[i]) {
            return s1[i] - s2[i];
        }
    }
    return 0;
}



/**
 * @brief compares the first n bytes of two strings s1 and s2.  
 * 
 * @param s1 : first source string
 * @param s2 : second source string
 * @param n : the size to compare 
 * @return int : result of the comparison
 *               0, if the s1 and s2 are equal
 *               a negative value if s1 is less than s2
 *               a positive value if s1 is greater than s2
 */
int strncmp(const char *s1, const char *s2, size_t n) {
    int i;
    for (i = 0; i < n; i++) {
        if ((s1[i] != s2[i]) || (s1[i] == '\0')) {
            return s1[i] - s2[i];
        }
    }
    return 0;
}



/**
 * @brief reverse the string s
 * 
 * @param s : source string
 * @return char* : pointer to the reversed s
 */
char *strrev(const char *s) {
    char *d = (char *)s;
    register char tmp;
    register size_t beg = 0;
    register size_t end = strlen(s) - 1;

    while (beg < end) {
        tmp = d[end];
        d[end] = d[beg];
        d[beg] = tmp;
        beg++;
        end--;
    }
    return d;
}



/**
 * @brief find the first occurrence of the character c in 
 * the string s
 * 
 * @param s : source string
 * @param c : character to find
 * @return char* : a pointer to the first occurrence of the
 * character c in the string s. NULL if not found.
 */
char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == c) {
            return (char *)s;
        }
        s++;
    }
    return NULL;
}



/**
 * @brief find the last occurrence of the character c in 
 * the string s
 * 
 * @param s : source string
 * @param c : character to find
 * @return char* : a pointer to the last occurrence of the
 * character c in the string s. NULL if not found.
 */
char *strrchr(const char *s, int c) {
    char *t1 = strrev(s);
    char *t2 = strchr(t1, c);
    return (char *) (strrev(s) + strlen(t2) - 1);
}



/**
 * @brief appends the src string to the dest string, overwriting 
 * the terminating null byte ('\0') at the end of dest, and then
 * adds a terminating null byte. The strings may not overlap, and 
 * the dest string must have enough space for the result. If dest 
 * is not large enough, program behavior is unpredictable.
 * 
 * @param dest : target string
 * @param src : source string
 * @return char* : result string 
 */
char *strcat(char * __restrict dest, const char * __restrict src) {
    while (*dest) dest++;
    return strcpy(dest, src);
}



/**
 * @brief appends the first n bytes from src string to the dest 
 * string, overwriting the terminating null byte ('\0') at the 
 * end of dest, and then adds a terminating null byte. The strings 
 * may not overlap, and the dest string must have enough space for 
 * the result. If dest is not large enough, program behavior is 
 * unpredictable.
 * 
 * @param dest : target string
 * @param src : source string
 * @param src :size to append
 * @return char* : result string 
 */
char *strncat(char * __restrict dest, const char * __restrict src, size_t n) {
    while (*dest) dest++;
    return strncpy(dest, src, n);
}



/**
 * @brief copies the string pointed to by src, including the 
 * terminating null byte ('\0'), to the buffer pointed to by dest.
 * The strings may not overlap, and the destination string dest must be
 * large enough to receive the copy. Beware of buffer overruns!  
 * 
 * @param dest : target string
 * @param src : source string
 * @return char* : pointer to dest
 */
char *strcpy(char * __restrict dest, const char * __restrict src) {
    char *tmp = dest;
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
    return tmp;
}



/**
 * @brief copies the first n bytes of string pointed to by src, including 
 * the terminating null byte ('\0'), to the buffer pointed to by dest.
 * The strings may not overlap, and the destination string dest must be
 * large enough to receive the copy. Beware of buffer overruns! 
 * 
 * @param dest : target string
 * @param src : source string
 * @param n : size to copy
 * @return char* : pointer to dest
 */
char *strncpy(char * __restrict dest, const char * __restrict src, size_t n) {
    int i = 0;
    while (src[i] != '\0' && i < n) {
        dest[i] = src[i];
        i++;
    }
    while (i < n) {
        dest[i] = '\0';
        i++;
    }
    return dest;
}



/**
 * @brief copies the string pointed to by src, including the 
 * terminating null byte ('\0'), to the buffer pointed to by dest.
 * The strings may not overlap, and the destination string dest must be
 * large enough to receive the copy. Beware of buffer overruns!  
 * 
 * @param dest : target string
 * @param src : source string
 * @return char* : pointer to dest's end
 */
char *stpcpy(char * __restrict dest, const char * __restrict src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
    return dest;
}



/**
 * @brief duplicate string
 * 
 * @param s : source string 
 * @return char* : a pointer to a new string which is a duplicate 
 * of the string s. Memory for the new string is obtained with 
 * malloc, and can be freed with free.
 */
char *strdup(const char *s) {
    size_t size = strlen(s);
    char *d = malloc(size + 1);
    return strcpy(d, s);
    return NULL;
}



/**
 * @brief 
 * 
 * @param s 
 * @param c 
 * @param n 
 * @return void* 
 */
void *memset(void *s, int c, size_t n) {
    c &= 0xFF;
    asm volatile ("                 \n\
            .memset_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memset_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memset_aligned \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memset_top     \n\
            .memset_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     stosl           \n\
            .memset_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memset_done    \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%edx       \n\
            jmp     .memset_bottom  \n\
            .memset_done:           \n\
            "
            :
            : "a"(c << 24 | c << 16 | c << 8 | c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}



/**
 * @brief copies n bytes from memory area src to memory area dest.  
 * The memory areas must not overlap.
 * 
 * @param dest : target string
 * @param src : source string
 * @param n : size to copy
 * @return void * : pointer to dst
 */
void *memcpy(void * __restrict dest, const void * __restrict src, size_t n) {
    asm volatile ("                 \n\
            .memcpy_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memcpy_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memcpy_aligned \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memcpy_top     \n\
            .memcpy_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     movsl           \n\
            .memcpy_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memcpy_done    \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%edx       \n\
            jmp     .memcpy_bottom  \n\
            .memcpy_done:           \n\
            "
            :
            : "S"(src), "D"(dest), "c"(n)
            : "eax", "edx", "memory", "cc"
    );
    return dest;
}



/**
 * @brief copies n bytes from memory area src to memory area dest.  
 * The memory areas could overlap.
 * (used for overlapping memory areas)
 * 
 * @param dest : target string
 * @param src : source string
 * @param n : size to copy
 * @return void * : pointer to dst
 */
void *memmove(void *dest, const void *src, size_t n) {
    asm volatile ("                             \n\
            movw    %%ds, %%dx                  \n\
            movw    %%dx, %%es                  \n\
            cld                                 \n\
            cmp     %%edi, %%esi                \n\
            jae     .memmove_go                 \n\
            leal    -1(%%esi, %%ecx), %%esi     \n\
            leal    -1(%%edi, %%ecx), %%edi     \n\
            std                                 \n\
            .memmove_go:                        \n\
            rep     movsb                       \n\
            "
            :
            : "D"(dest), "S"(src), "c"(n)
            : "edx", "memory", "cc"
    );
    return dest;
}



/**
 * @brief Convert a number to its ASCII representation, with base "radix"
 * 
 * number to convert
 *            int8_t* buf = allocated buffer to place string in
 *          int32_t radix = base system. hex, oct, dec, etc.
 * Return Value: number of bytes written
 * @param value : number to convert
 * @param buf : allocated buffer to place string in
 * @param radix :  base system. hex, oct, dec, etc.
 * @return char* : number of bytes written
 */
char *itoa(unsigned int value, char *buf, int radix) {
    static char lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char *newbuf = buf;
    int i;
    unsigned int newval = value;

    /* Special case for zero */
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }

    /* Go through the number one place value at a time, and add the
     * correct digit to "newbuf".  We actually add characters to the
     * ASCII string from lowest place value to highest, which is the
     * opposite of how the number should be printed.  We'll reverse the
     * characters later. */
    while (newval > 0) {
        i = newval % radix;
        *newbuf = lookup[i];
        newbuf++;
        newval /= radix;
    }

    /* Add a terminating NULL */
    *newbuf = '\0';

    /* Reverse the string and return */
    return strrev(buf);
}



/**
 * @brief converts the initial portion of the string pointed to by nptr to int.
 * 
 * @param nptr : string representation of a number
 * @return int : the numebr
 */
int atoi(const char *nptr) {
    int number = 0;
    int sign = 1;

    /* check sign */
    if (*nptr == '-') {
        sign = -1;
        nptr++;
    }

    while (*nptr) {
        if (*nptr >= '0' && *nptr <= '9')
            number = number * 10 + (*nptr - '0');
        else 
            break;
        nptr++;
    }

    return sign * number;
}
