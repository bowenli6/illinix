#ifndef _ACCESS_H_
#define _ACCESS_H_

#include <types.h>

#define MAXADDR ((1 << 32) - 1)


int32_t validate_addr(void *addr);
int32_t copy_from_user(void *to, const void *from, uint32_t n);
int32_t copy_to_user(void *to, const void *from, uint32_t n);


#endif /* _ACCESS_H_ */