#include <stdio.h>
#include "string.h"
#include <stdlib.h>
#include <unistd.h>

#define PROMASK 0xffffe000   

#define GETPRO(p)                       \
do {                                    \
    asm volatile ("                   \n\
            movl %1, %%ecx            \n\
            andl %%esp, %%ecx         \n\
            movl %%ecx, %0            \n\
            "                           \
            : "=r"(p)                   \
            : "r" (PROMASK)             \
            : "memory"                  \
    );                                  \
} while (0)

int main(void) {
    void *p;
    GETPRO(p);
    return 0;
}   
