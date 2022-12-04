#ifndef _ACCESS_H_
#define _ACCESS_H_

#include <types.h>

#define MAXADDR ((1 << 32) - 1)
#define PROMASK 0xffffe000   
#define KERNEL_PRESERVED  0x100000
#define USER_STACK_ADDR   (0xC000000 - 4)
#define USER_STACK_MAX    0x400000
#define PROGRAM_IMG_BEGIN 0x08048000     
#define VIR_MEM_BEGIN     0x08000000 

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

int32_t copy_from_user(void *to, const void *from, uint32_t n);
int32_t copy_to_user(void *to, const void *from, uint32_t n);

void *alloc_kstack(void);
void free_kstack(void* pt);


#endif /* _ACCESS_H_ */
