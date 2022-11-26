#include <stdio.h>
#include "string.h"
#include <stdlib.h>
#include <unistd.h>

typedef long long int64_t;
typedef unsigned long long uint64_t;

#define PROMASK 0xffffe000   

typedef struct _rb_node {
    unsigned int rb_parent_color;
    struct _rb_node *right;
    struct _rb_node *left;
} rb_node;


/* sched process info */
typedef struct {
    uint64_t vruntime;          /* for how long this entity has run on a CPU, based on a virtual clock */
    uint64_t exec_start;        /* time when the process start running */
    uint64_t sum_exec_time;     /* time the process has been running in total */
    uint64_t prev_sum_exec_time;/* used for storing the previous run time of a process */
    rb_node  node;              /* a red-block tree node for this thread */
    int8_t   on_rq;             /* does the process on runqueue now? */
} sched_t;

typedef struct thread {
    sched_t            sched_info;      /* info used for scheduler */
    int32_t            argc;            /* number of arguments */
    int8_t             **argv;          /* user command line argument */
} thread_t;

#define container_of(ptr, type, member) ({		    \
	void *__mptr = (void *)(ptr);					\
	((type *)(__mptr - offsetof(type, member))); })


#define offsetof(type, member) \
    ((unsigned int)((unsigned char*)&((type*)0)->member - (unsigned char*)0))


#define task_of(ptr)  container_of(ptr, thread_t, sched_info)


/* weight info */
typedef struct {
    unsigned int weight;
    unsigned int inv_weight;
} weight_t;

static inline update_load(weight_t *load, unsigned int weight) {
    load->weight += weight;
}

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
    weight_t *myload = malloc(sizeof(weight_t));
    myload->weight = 123;
    weight_t load = *myload;
    myload->weight = 100;
    printf("%d\n", (int)load.weight);
    return 0;
}   
