/**
 * @file alloc.c
 * @author Bowen Li (bowenli6@illinois.edu)
 * @brief Dynamic memory allocator
 * @version 0.1
 * @date 2022-10-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "alloc.h"

/* Private global variables */

/* It stores eight linked lists of block_t. Each linked list 
 * repersents one data size, from 2^5 (index 0) to 2^12 (index 7).
 * Any blocks hold more than CHUNK_SIZE will not be stored in this
 * list. 
 */
block_t **free_list;

static unsigned init_flag = 0;      /* zero if the free_list need to be initialized*/


/* Private helper functions. */

static void *Sbrk(size_t size);
static void init_free_list(void);
static void *__alloc(int index);
static void pack(block_t *block, size_t size, unsigned used);
static void fl_add(block_t *block, int index);
static block_t *fl_remove(int index);
static inline __attribute__((unused)) int block_index(size_t x);



/**
 * @brief Allocates size bytes and returns a pointer to the
 * allocated memory. The memory is not initialized. If size 
 * is 0, then malloc() returns either NULL, or a unique 
 * pointer value that can later be successfully passed to 
 * free().
 * 
 * @param size : The number of bytes to allocate.
 * @return void* : Success : The address of the allocated memory.
 *                 Failure : NULL.
 */
void *malloc(size_t size) {
    if (size <= 0) return NULL;

    /* Initialize free list when flag is zero. */
    if (!init_flag) {
        init_flag = 1;
        fprintf(stderr, "size = %d\n", (int) size);
        init_free_list();
    }   

    

    return NULL;
}


/**
 * @brief The calloc() function allocates memory for an array 
 * of count elements of size bytes each and returns a pointer 
 * to the allocated memory. The memory is set to zero. If count 
 * or size is 0, then calloc() return either  NULL, or a unique 
 * pointer value that can later be successfully passed to free().  
 * If the multiplication of count and size would result in integer 
 * overflow, then calloc() returns an error. By contrast, an integer 
 * overflow would not be detected in the following call to malloc(),  
 * with the result that an incorrectly sized block of memory would
 * be allocated: malloc(count * size);
 * 
 * @param count : The number of elements.
 * @param size : The number of bytes for each element.
 * @return void* : Success : The address of the allocated memory.
 *                 Failure : NULL.
 */
void *calloc(size_t count, size_t size) {
    void *ptr;
    size_t nbytes = count * size;
    if (count != 0 && nbytes / count != size) {
        fprintf(stderr, "Multiplication of count and size result in integer overflow.\n");
        return NULL;
    }
    ptr = malloc(nbytes);
    memset(ptr, 0, nbytes);
    return ptr;
}


/**
 * @brief The realloc() function changes the size of the memory 
 * block pointed to by ptr to size bytes. The contents will be 
 * unchanged in the range from the start of the region up to the 
 * minimum of the old and new sizes. If the new size is larger 
 * than the old size, the added memory will not be initialized.  
 * If ptr is NULL, then the call is equivalent to malloc(size), 
 * for all values of size; if size is equal to zero, and ptr is 
 * not NULL, then the call is equivalent to free(ptr).  Unless  
 * ptr is NULL, it must have been returned by an earlier call to 
 * malloc(), calloc(), or realloc().  If the area pointed to was 
 * moved, a free(ptr) is done.
 * 
 * @param ptr : The old address of the allocated memory.
 * @param size : The new size of the memory.
 * @return void* : Success : The address of the allocated memory.
 *                 Failure : NULL.
 */
void *realloc(void *ptr, size_t size) {
    return NULL;
}


/**
 * @brief The free() function frees the memory space pointed to 
 * by ptr, which must have been returned by a previous call to 
 * malloc(), calloc(), or realloc(). Otherwise, or if free(ptr) 
 * has already been called before, undefined behavior occurs.  
 * If ptr is NULL, no operation is performed.
 * 
 * @param ptr : The address of the allocated memory to be freed.
 */
void free(void *ptr) {
    if (!ptr) return;
}


/**
 * @brief Allocates a contiguous memory region of at least size 
 * bytes. It MAY NOT BE USED as the allocator for pool-allocated
 * regions. Memory allocated using bulk_alloc() must be freed by
 * bulk_free()
 * 
 * @param size : The number of bytes to allocate.
 * @return void* : Success : The address of the allocated memory.
 *                 Failure : NULL.
 */
void *bulk_alloc(size_t size) {
    void *map;
    if ((map = mmap(NULL, size, PROT_READ | PROT_WRITE, 
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED)
        return NULL;
    return map;
    
}

/**
 * @brief Frees an allocation created with bulk_alloc(). Note that
 * the pointer passed to this function MUST have been returned by
 * bulk_alloc(), and the size MUST be the same as the size passed
 * to bulk_alloc() when that memory was allocated. Any other usage
 * is likely to fail, and may crash your program.
 * 
 * @param ptr : The address of the allocated memory to be freed. 
 * @param size : The size of the allocated memory to be freed.
 */
void bulk_free(void *ptr, size_t size) {
    if (munmap(ptr, size)) {
        fprintf(stderr, "munmap failed in bulk_free(); \
            you probably passed invalid arguments.\n");
    }
}


static void *Sbrk(size_t size) {
    void *ptr;
    if ((ptr = sbrk(CHUNK_SIZE)) == (void *)-1) {
        fprintf(stderr, "Failed to allocate memory from the heap.\n");
        return NULL;
    }
    return ptr;
}


/**
 * @brief Initialize the free list.
 * 
 */
static void init_free_list(void) {
    int i;          
    int index;              /* Index of the remaining data we need for free list. */
    int size;               /* Size of the user block. */
    void *ptr;              /* Points to new allocated memory address. */
    block_t **bp;           /* Block pointer used for free_list. */

    
    ptr = Sbrk(CHUNK_SIZE);
    bp = (block_t **)ptr;
    free_list = bp;
    memset((void *)free_list, 0, FSSIZE);
    bp += FSSIZE;
    size = CHUNK_SIZE - FSSIZE * sizeof(block_t *);
    index = block_index(size - DSIZE);
    pack((BLOCK(bp)), size, FREED);       
    fl_add((BLOCK(bp)), index);  
}


static void *__alloc(int index) {
    void *ptr;              /* Points to new allocated memory address. */    
    int size;               /* Size of the user block. */
    int index_remain;       /* Index of the remaining data we need for free list. */
    int size_remain;        /* Size of the remaining block. */
    block_t *block;         /* Block struct used for packing. */

    ptr = Sbrk(CHUNK_SIZE);

    /* Pack user block's header and footer. */
    block = BLOCK(ptr);
    size = 1 << index;

    pack(block, size, USED);
    fl_add(block, index);           /* Add block into free_list. */

    /* Pack remaining block's header and footer. */
    block = BLOCK(ptr + size);
    index_remain = block_index(CHUNK_SIZE - size - DSIZE);
    size_remain = 1 << index_remain;
    pack(block, size_remain, FREED);
    fl_add(block, index_remain);     /* Add block into free_list. */

    return ptr + WSIZE;
}


static void pack(block_t *block, size_t size, unsigned used) {
    fprintf(stderr, "%p\n", block);
    fprintf(stderr, "%d\n", (int)size);

    PUT(block->header, PACK(size, used));
    PUT(block->footer, PACK(size, used));
}


static void fl_add(block_t *block, int index) {

}

static block_t *fl_remove(int index) {
    return NULL;
}


/**
 * @brief This function computes the log base 2 of the allocation block 
 * size for a given allocation. To find the allocation block size from 
 * the result of this function, use 1 << block_size(x).
 *
 * Note that its results are NOT meaningful for any
 * size > 4088!
 *
 * If you are curious, see the gcc info page and search for __builtin_clz; 
 * it basically counts the number of leading binary zeroes in the value
 * passed as its argument.
 * 
 * @param x : The size to be computed.
 * @return int : Log base 2 of the allocation block size for a given allocation.
 * 
 */
static inline __attribute__((unused)) int block_index(size_t x) {
    if (x <= 16) {
        return 5;
    } else {
        return 32 - __builtin_clz((unsigned int)x + 15);
    }
}   


int main(void) {
    fprintf(stderr, "%d\n", block_index(17));
}


