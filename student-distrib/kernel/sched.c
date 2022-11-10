#include <pro/sched.h>
#include <pro/process.h>
#include <lib.h>


/**
 * @brief only schedule the init process now
 * 
 * @return int32_t : 0
 */
int32_t schedule() {
    init_task();
    return 0;
}



/**
 * @brief halts the central processing unit (CPU) until 
 * the next external interrupt is fired.
 * 
 */
void pause() {
    sti(); /* enable interrupts */

    /* Spin (nicely, so we don't chew up cycles) */
    asm volatile (".1: hlt; jmp .1;");
}