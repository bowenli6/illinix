#ifndef _STDLIB_H_
#define	_STDLIB_H_

#include "types.h"

void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

#endif /* _STDLIB_H_ */
