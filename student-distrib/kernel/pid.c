#include <pro/pid.h>
#include <types.h>
#include <errno.h>
#include <lib.h>

// uint8_t pidmap[BITMAP_SIZE];
pid_t size;

/**
 * @brief Allocate a new process id for the current process
 * 
 * @return pid_t : The process id
 */
pid_t alloc_pid() {
    int i;
    // int i, j, mask;
    
    if (size < BITMAP_SIZE) {
        i = size++;
        return i;
    }

    // mask = 0x1;
    // for (i = 0; i < BITMAP_SIZE; ++i) {
    //     uint8_t pid_group = pidmap[i];
    //     j = 0;

    //     while (j < 8) {
    //         if (!(pid_group & (mask << j))) {
    //             return i * 8 + j;
    //         }
    //         j++;
    //     }
    // }

    /* resource temporarily unavailable */
    return -EAGAIN;
}


/**
 * @brief init the pidmap, calling only by the init process
 * 
 */
void pidmap_init() {
    // memset((void *)pidmap, 0, BITMAP_SIZE);
    size = 2;   /* 0, 1 are allocted by idle and init process */
}


/**
 * @brief Kill the current pid
 * 
 * @return return the previous pid
 */
pid_t kill_pid() {
    if (size > 2)
        return --size;
    
    return -1;

}


