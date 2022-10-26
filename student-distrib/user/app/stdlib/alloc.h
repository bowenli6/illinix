#ifndef _ALLOC_H
#define _ALLOC_H

#include <unistd.h>

/* When requesting memory from the OS using sbrk(),
 * request it in increments of CHUNK_SIZE. */
#define CHUNK_SIZE (1 << 12)

/* Standard API */
void *malloc(size_t size);
void *calloc(size_t count, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
void *bulk_alloc(size_t size);
void bulk_free(void *ptr);

#endif  /* _ALLOC_H */
