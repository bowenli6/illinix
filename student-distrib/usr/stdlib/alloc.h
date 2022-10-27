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
#define USED        1               /* Block is in used. */
#define FREED       0               /* Block is freed. */
#define WSIZE       4               /* Word and header/footer size (bytes) */
#define DSIZE       8               /* Double word size (bytes) */
#define FSSIZE      8               /* The number of elements in the free list. */
#define CHUNK_SIZE  (1 << 12)       /* When requesting memory from the kernel using sbrk(), 
                                     * request it in increments of CHUNK_SIZE: 4KB. */

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word. */
#define PACK(size, alloc)   ((size) | (alloc))

/* Read and write a word at address p. */
#define GET(p)              (*(size_t *) (p))
#define PUT(p, val)         (*(size_t *) (p) = (val))

/* Read the size and allocated fields from address p. */
#define GET_SIZE(p)         (GET(p) & ~0x7)
#define GET_ALLOC(p)        (GET(p) & 0x1)

/* Given a void pointer, returns the block_t pointer. */
#define BLOCK(p)            ((block_t *) (p))

/* Given block ptr bp, compute address of its header, footer, and data. */
#define HDRP(bp)            ((char *) (bp) - WSIZE)
#define FTRP(bp)            ((char *) (bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given header ptr hd, compute address of its data. */
#define DATA(hd)            ((char *) (hd) + WSIZE)

/* Given block ptr bp, compute address of next and previous blocks. */
#define NEXT_BLKP(bp)       ((char *) (bp) + GET_SIZE(((char *) (bp) - WSIZE)))
#define PREV_BLKP(bp)       ((char *) (bp) - GET_SIZE(((char *) (bp) - DSIZE)))


/* */


/* A ptr_t is a struct of two pointers. The prev points to its 
 * previous block within the current free list. The next points to 
 * its next block within the current free list.
 */
typedef struct {
    void *prev;
    void *next;
} ptr_t;


/* A data_t uses one space for two type of data. When the memory block 
 * is in used by the user, the user will access the data part. When the 
 * memory block is unused, the allocator will acccess the ptr part for 
 * managing blocks in the free list.
*/
typedef union {
    void *data;
    ptr_t ptr;
} data_t;


/* A block_t is a memory block allocated for the user. It has a header
 * and a footer which store the size of the block and one bit to indicate 
 * whether the block is currently in used. */
typedef struct {
    size_t header;
    data_t data;
    size_t footer;
} block_t; 


/* Standard API */
void *malloc(size_t size);
void *calloc(size_t count, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
void *bulk_alloc(size_t size);
void bulk_free(void *ptr, size_t size);

#endif  /* _ALLOC_H */
