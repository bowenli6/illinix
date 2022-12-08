#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define START 5     // 5 is the first size of base 2 allocation can be made.
#define END 12      // 12 is the last size of base 2 allocation can be made.
#define LEN 8       // length of the void **index.
#define LAST_BIT 31 // index of the last bit in X86-32.


/* When requesting memory from the OS using sbrk(), request it in
 * increments of CHUNK_SIZE. */
#define CHUNK_SIZE (1<<12)


// define a free list structure.
typedef struct {
    void *heap;            // a pointer points to the brk point.
    void **index;           // an array of pointers point to the free linked lists.
    size_t size_usable;     // the possible usage of size in the heap.
} List;


static List *free_list = NULL;


static void *bulk_alloc(size_t size);
static void bulk_free(void *ptr, size_t size);
static inline __attribute__((unused)) int block_index(size_t x);
static void add(List *free_list, int idx, void* ptr);
static void* drop(List *free_list, int idx);
static void set_flag(void *ptr);
static void clear_flag(void *ptr);
static int check_flag(void *ptr);
static void set_up_free_list();
static void *allocate(size_t size);
static void transferring_data(void *src, void *dst, size_t size);



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
    return allocate(size);
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
void *calloc(size_t nmemb, size_t size) {
    void *ptr = allocate(nmemb * size);
    memset(ptr, 0, nmemb * size);
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
    void *new_ptr;
    size_t *size_ptr;
    void *next_block;
    size_t size_needed;

    if (ptr == NULL) {
        return malloc(size);
    } else if (size == 0) {
        new_ptr = malloc(1);
        transferring_data(ptr, new_ptr, 24);
        free(ptr);
        return new_ptr;
    }

    size_ptr = (size_t*)(ptr - sizeof(size_t));
    clear_flag((void*)size_ptr);
    if (*size_ptr - sizeof(size_t) >= size) {
        set_flag((void*)size_ptr);
        return ptr;
    }
    
    // needs to realloc
    next_block = ptr + *size_ptr - sizeof(size_t);
    size_needed = size - (*size_ptr - sizeof(size_t));
    
    if (next_block == free_list -> heap) {
        if (free_list -> size_usable >= (1 << block_index(size_needed))) {
            new_ptr = malloc(size_needed);
            *size_ptr += *(size_t*)(new_ptr - sizeof(size_t));
            set_flag(size_ptr);
            return ptr;
        }
    }

    new_ptr = malloc(size);
    transferring_data(ptr, new_ptr, *size_ptr - sizeof(size_t));
    free(ptr);
    return new_ptr;
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
    size_t size;

    if (ptr == NULL) return;
    if (check_flag(ptr - sizeof(size_t))) clear_flag(ptr - sizeof(size_t));

    size = *(size_t*)(ptr - sizeof(size_t));

    int idx = block_index(size - sizeof(size_t));

    if (idx > END) {
        bulk_free(ptr - sizeof(size_t), size);
    } else {
        add(free_list, idx - START, ptr);
    }
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
    void *mapping =  mmap(NULL, size);

    if (mapping == (void*)-1) {
        return NULL;
    } else {
        return mapping;
    }
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
        fprintf(stderr, "munmap failed in bulk_free(); you probably passed invalid arguments\n");
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


// add node to the head of the linkedList.
static void add(List *free_list, int idx, void* ptr) {
    *(void**)ptr = free_list -> index[idx];
    free_list -> index[idx] = ptr;
}


// drop node to the head of the linkedList.
static void* drop(List *free_list, int idx) {
    void *ptr = free_list -> index[idx];
    free_list -> index[idx] = *(void**)ptr;
    return ptr;
}


// set the last bit of an size_t to 1.
static void set_flag(void *ptr) {
    *(size_t*)ptr |= 1UL << LAST_BIT;
}

// clear the last bit of an size_t to 0.
static void clear_flag(void *ptr) {
    *(size_t*)ptr &= ~(1UL << LAST_BIT);
}

// return the last bit of an size_t.
static int check_flag(void *ptr) {
    return (*(size_t*)ptr >> LAST_BIT) & 1U;
}



// set up the free list by using sbrk().
static void set_up_free_list() {
    void *heap = sbrk(CHUNK_SIZE);
    free_list = heap;
    heap += sizeof(List);
    free_list -> index = heap;
    heap += LEN * sizeof(void*);
    free_list -> heap = heap + 40;
    memset(free_list -> index, 0, LEN * sizeof(void*));
    free_list -> size_usable = CHUNK_SIZE - 128;
}


// allocate an block of memory of size_t to the user.
static void *allocate(size_t size) {
    void *ptr;
    int block;
    size_t chunk_size;
    int index, idx;
    void *p;

    if (size <= 0) return NULL;    // no allocation made.
    if (free_list == NULL) set_up_free_list();
    
    block = block_index(size);
    chunk_size = free_list -> size_usable;
    size = 1 << block;
    if (chunk_size < size) {
        index = block_index(chunk_size - 8);
        chunk_size = 1 << index;
        if (chunk_size >= 32) {
            p = free_list -> heap;
            *(size_t*)p = chunk_size;
            add(free_list, index, p + sizeof(size_t));
        }
        free_list -> heap = sbrk(CHUNK_SIZE);
        free_list -> size_usable = CHUNK_SIZE;
    }
    if (block > END) {
        ptr = bulk_alloc(size + sizeof(size_t));
        *(size_t*)ptr = size + sizeof(size_t);
        set_flag(ptr);
        ptr += sizeof(size_t);
    } else {
        idx = block - START;
        if (free_list -> index[idx] != NULL) {
            ptr = drop(free_list, idx);
        } else {
            ptr = free_list -> heap + sizeof(size_t);
            free_list -> heap += size;
            free_list -> size_usable -= size;
        }
        *(size_t*)(ptr - sizeof(size_t)) = size;
        set_flag(ptr - sizeof(size_t));
    }
    return ptr;
}

static void transferring_data(void *src, void *dst, size_t size) {
    int i;
    for (i = 0; i < size; i++) {
        ((char*)dst)[i] = ((char*)src)[i];
    }
}


/**
 * @brief exit the process
 * 
 * @param status : status message
 */
void exit(int status) {
    _exit(status);
}


/**
 * @brief Stevens-style error printing for a Unix-style error
 * 
 * @param msg : error message
 */
static void unix_error(char *msg) {
    printf("%s\n", msg);
    exit(0);
}


/**
 * @brief Stevens-style error-handling wrapper function for fork
 * 
 * @return pid_t : 0 to the child process
 *                 pid to the parent process
 */
pid_t Fork(void) {
    pid_t pid;

    if ((pid = fork()) < 0)
        unix_error("Fork failed");

    return pid;
}

/**
 * @brief Stevens-style error-handling wrapper function for execv
 * 
 */
void Execv(const char *pathname, char *const argv[]) {
    if (execv(pathname, argv) < 0)
        unix_error("Command not found");
}


/**
 * @brief Stevens-style error-handling wrapper function for waitpid
 * 
 * @param pid : child process id to wait
 * @param wstatus : child process status
 */
void Waitpid(pid_t pid, int *wstatus) {
    if (waitpid(pid, wstatus) < 0) {
        unix_error("waitfg: waitpid failed");
    }
}

