#include <boot/signal.h>
#include <boot/interrupt.h>
#include <boot/exception.h>
#include <pro/pid.h>
#include <pro/sched.h>
#include <lib.h>

static int32_t add_mask(int signum);
static int32_t remove_mask(int signum);
static int32_t div_zero_handler();
static int32_t segfault_handler();
static int32_t interrupt_handler();
static int32_t alarm_handler(); 
static int32_t user1_handler();




asmlinkage int32_t sys_set_handler(int32_t signum, void *handler_addr) {
    if (signum >= SIG_COUNT || signum < 0) return -1;
   
    int flags;
    cli_and_save(flags);
    if (handler_addr == NULL) {
        CURRENT->sig->exe_sig_act[signum] = default_arr[signum];
    } else {
        CURRENT->sig->exe_sig_act[signum] = (default_action*) handler_addr;
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
    if (signum >= SIG_COUNT || signum < 0) return -1;

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
    if (signum >= SIG_COUNT || signum < 0) return -1;
    int flags;
    cli_and_save(flags);

    CURRENT->sig->mask_arr[signum] = 0;
    
    restore_flags(flags);
    return 0;
}

int32_t send_signal(thread_t* thread, int8_t signum) {
    if (thread == NULL) return -1;

    if (signum >= SIG_COUNT || signum < 0) return -1;

    int flags;
    cli_and_save(flags);

    if (thread->sig->pen_arr[signum] == 0) {
        thread->sig->pen_arr[signum] = 1;
        thread->sig->pending_sig_count += 1;
    } 

    restore_flags(flags);
    return 0;
}


static
int32_t div_zero_handler() {
    sys_halt(256);
    return 0;
}

static
int32_t segfault_handler() {
    sys_halt(256);
    return 0;
}

static
int32_t interrupt_handler() {
    return 0;
}

static
int32_t alarm_handler() {
    return 0;
}

static 
int32_t user1_handler () {
    return 0;
}

int32_t sig_init(){
    default_arr[DIV_ZERO] = &div_zero_handler;
    default_arr[SEGFAULT] = &div_zero_handler;
    default_arr[INTERRUPT] = &interrupt_handler;
    default_arr[ALARM] = &alarm_handler;
    default_arr[USER1] = &user1_handler;
    
    return 0;
}

int32_t thread_sig_init(thread_t* thread) {
    int i;

    if (thread == NULL) return -1;
    
    thread->sig->curr_sig = -1;
    thread->sig->pending_sig_count = 0;

    for (i = 0; i < SIG_COUNT; ++ i) {
        thread->sig->mask_arr[i] = 0;
        thread->sig->pen_arr[i] = 0;
        thread->sig->exe_sig_act[i] = (default_action*)NULL;
    }

    return 0;
}