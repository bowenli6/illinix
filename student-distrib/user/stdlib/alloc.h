#ifndef _ALLOC_H
#define _ALLOC_H

/**
 * @file alloc.h
 * @author Bowen Li (bowenli6@illinois.edu)
 * @brief Dynamic memory allocator
 * @version 0.1
 * @date 2022-10-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <unistd.h>

/* Basic constants and macros */
#define WSIZE       4               /* Word and header/footer size (bytes) */
#define DSIZE       8               /* Double word size (bytes) */
#define CHUNK_SIZE  (1 << 12)       /* When requesting memory from the OS using sbrk(), 
                                     * request it in increments of CHUNK_SIZE: 4KB. */

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word. */
#define PACK(size, alloc)   ((size) | (alloc))

/* Read and write a word at address p. */
#define GET(p)              (*(unsigned int *) (p))
#define PUT(p, val)         (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p. */
#define GET_SIZE(p)         (GET(p) & ~0x7)
#define GET_ALLOC(p)        (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer. */
#define HDRP(bp)            ((char *) (bp) - WSIZE)
#define FTRP(bp)            ((char *) (bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks. */
#define NEXT_BLKP(bp)       ((char *) (bp) + GET_SIZE(((char *) (bp) - WSIZE)))
#define PREV_BLKP(bp)       ((char *) (bp) - GET_SIZE(((char *) (bp) - DSIZE)))


/* Standard API */
void *malloc(size_t size);
void *calloc(size_t count, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
void *bulk_alloc(size_t size);
void bulk_free(void *ptr, size_t size);

#endif  /* _ALLOC_H */
