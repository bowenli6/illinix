#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "alloc.h"

/* Local helper functions. */

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
    /* TODO: Check overflow */
    void *ptr = malloc(count * size);
    memset(ptr, 0, count * size);
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
    if (map = mmap(NULL, size, PROT_READ | PROT_WRITE, 
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0) == MAP_FAILED)
        return NULL;
    else
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
    if (x <= 8) {
        return 5;
    } else {
        return 32 - __builtin_clz((unsigned int)x + 7);
    }
}   
