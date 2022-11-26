/* types.h - Defines to use the familiar explicitly-sized types in this
 * OS (uint32_t, int8_t, etc.).  This is necessary because we don't want
 * to include <stdint.h> when building this OS
 * vim:ts=4 noexpandtab
 */

#ifndef _TYPES_H
#define _TYPES_H

#define NULL 0

#ifndef ASM

/* Types defined here just like in <stdint.h> */
typedef long long int64_t;
typedef unsigned long long uint64_t;

typedef int int32_t;
typedef unsigned int uint32_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef char int8_t;
typedef unsigned char uint8_t;

typedef uint32_t pid_t;
typedef uint32_t gid_t;

/* x is likely to be true */
#define likely(x)	    __builtin_expect(!!(x), 1)

/* x is unlikely to be treu */
#define unlikely(x)	    __builtin_expect(!!(x), 0)

#endif /* ASM */

#endif /* _TYPES_H */
