#include <boot/signal.h>
#include <boot/interrupt.h>
#include <boot/exception.h>
#include <pro/pid.h>
#include <pro/sched.h>
#include <lib.h>
#include <pro/process.h>

static int32_t add_mask(int signum);
static int32_t remove_mask(int signum);
static int32_t div_zero_handler();
static int32_t segfault_handler();
static int32_t interrupt_handler();
static int32_t alarm_handler();     
static int32_t user1_handler();

int32_t thread_sig_init(process_t *thread);

int32_t send_signal(process_t *thread, int8_t signum);

/** 
 * @brief occurs when Kernel ----> Usr 
 *   
 * @return return 0 indicates succeed, otherwise -1
*/
asmlinkage int32_t deliver_signal() {
    /* First mask all signal and store the original stuff */ 
    int32_t i, flags, sig_num;

    cli_and_save(flags);

    /* store the previous mask state and then mask all other signals */
    for (i = 0; i < SIG_COUNT; ++ i) {
        CURRENT->sig->previous_mask_arr[i] = CURRENT->sig->mask_arr[i];
        add_mask(i);
    }

    /* mask all signal*/
    for (i = 0; i < SIG_COUNT; ++ i) {
        if (CURRENT->sig->mask_arr[i] != 1 && CURRENT->sig->pen_arr[i] == 1) {
            sig_num = i;
            break;
        }
    }

    /* According to ULK P.441, if ka.sa.sa_handler is equal to ISG_DFL, we must perform default handler */
    if (CURRENT->sig->exe_sig_act[sig_num] == default_arr[sig_num]) {
        default_arr[sig_num](); 

        /*unmask and change previous_mask to the initial state */
        for (i = 0; i < SIG_COUNT; ++ i) {
            CURRENT->sig->mask_arr[i] = CURRENT->sig->previous_mask_arr[i];
            CURRENT->sig->previous_mask_arr[i] = 0;
        }   
    } else {
        do_deliver(CURRENT->context, CURRENT->sig->exe_sig_act[sig_num], sig_num);
    }

    restore(flags);
    return 0;
}

/**
 * @brief set the sigreturn, using assmbly to set the stack 
 * @return return 0 indicates succeed, otherwise -1
*/
asmlinkage int32_t sys_sigreturn(void) {
   do_syssig_return();
   return 0;
}

/**
 * @brief syscall for set specific signal handler to the Current thread
 * 
 * @return return 0 if successfull
*/
asmlinkage int32_t sys_set_handler(int32_t signum, void *handler_addr) {
    if (signum >= SIG_COUNT || signum < 0) return -1;
   
    int flags;
    cli_and_save(flags);
    if (handler_addr != NULL) {
        CURRENT->sig->exe_sig_act[signum] = (default_action) handler_addr;
    } else {
        CURRENT->sig->exe_sig_act[signum] = default_arr[signum];
    }
    restore_flags(flags);

    return 0;
}

/**
 * @brief add the mask of corrosponding signal
 * 
 * 
*/
static
int32_t add_mask(int signum) {
    if (signum >= SIG_COUNT || signum < 0) return -1;

    int flags;
    cli_and_save(flags);

    CURRENT->sig->mask_arr[signum] = 1;

    restore_flags(flags);
}

/**
 * @brief remove the mask of corrosponding signal
 * 
 * 
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

/**
 * @brief when signal send to the process, record the signal 
 * 
 * 
*/
int32_t send_signal(process_t *thread, int8_t signum) {
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
    do_halt(256);
    return 0;
}

static
int32_t segfault_handler() {
    do_halt(256);
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

/**
 * @brief initialize the default handler 
 * 
 * 
*/
int32_t sig_init(){
    default_arr[DIV_ZERO] = &div_zero_handler;
    default_arr[SEGFAULT] = &div_zero_handler;
    default_arr[INTERRUPT] = &interrupt_handler;
    default_arr[ALARM] = &alarm_handler;
    default_arr[USER1] = &user1_handler;
    
    return 0;
}


/**
 * @brief initialize the intial state of signal_struct_t of each thread 
 * 
 * 
*/
int32_t thread_sig_init(process_t *thread) {
    int i;

    if (thread == NULL) return -1;
    
    thread->sig->curr_sig = -1;
    thread->sig->pending_sig_count = 0;

    for (i = 0; i < SIG_COUNT; ++ i) {
        thread->sig->mask_arr[i] = 0;
        thread->sig->pen_arr[i] = 0;
        thread->sig->previous_mask_arr[i] = 0;
        thread->sig->exe_sig_act[i] = (default_action)NULL;
    }

    return 0;
}