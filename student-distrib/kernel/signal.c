#include <boot/signal.h>
#include <boot/interrupt.h>
#include <boot/exception.h>
#include <pro/pid.h>
#include <pro/process.h>
#include <pro/sched.h>
#include <lib.h>

static int32_t add_mask(int signum);
static int32_t remove_mask(int signum);
static int32_t div_zero_handler();



asmlinkage int32_t sys_set_handler(int32_t signum, void *handler_addr) {
    if (signum >= SIG_COUNT || signum < 0) return -1;
   
    int flags;
    cli_and_save(flags);
    if (handler_addr == NULL) {
        CURRENT->sig->exe_sig_act[signum] = default_action[signum];
    } else {
        CURRENT->sig->exe_sig_act[signum] = handler_addr;
    }
    restore_flags(flags);

    return 0;
}

asmlinkage int32_t sys_sigreturn(void) {
   return 0;
}


/* 
    add signal to the process
*/
static
int32_t add_mask(int signum) {
    if (signum >= COMMAND_COUNT || signum < 0) return -1;

    int flags;
    cli_and_save(flags);

    CURRENT->sig->mask_arr[signum] = 1;

    restore_flags(flags);
}

/* 
    remove signal to the process
*/
static 
int32_t remove_mask(int signum) {
    if (signum >= COMMAND_COUNT || signum < 0) return -1;
    int flags;
    cli_and_save(flags);

    CURRENT->sig->mask_arr[signum] = 0;
    
    restore_flags(flags);
    return 0;
}

static
int32_t div_zero_handler() {
    int flags;
    cli_and_save(flags);

    
    
    restore_flags(flags);
}

int32_t sig_init(){
    default_action[DIV_ZERO] =;
    default_action[SEGFAULT] =;
    default_action[INTERRUPT] =;
    default_action[ALARM] =;
    default_action[USER1] =;

}