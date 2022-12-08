#include <pro/process.h>
#include <pro/signal.h>
#include <boot/syscall.h>



default_action default_arr[SIG_COUNT];

static int32_t div_zero_hdler(void);
static int32_t segfault_hler(void);
static int32_t interrupt_hdler(void);
static int32_t alarm_hdler(void);     
static int32_t user1_hdler(void);       

static
int32_t div_zero_hdler(void){
    do_exit(256);
    return 0;
}

static
int32_t segfault_hdler(void){
    do_exit(256);
    return 0;
}

static
int32_t interrupt_hdler(void){
    do_exit(256);
    return 0;
}

static
int32_t alarm_hdler(void){
    return 0;
}

static 
int32_t user1_hdler(void){
    return 0;
}


/**
 * @brief sys_sigreturn syscall, there will be a parameter telling the usr_esp register 
 * 
*/
asmlinkage int32_t sys_sigreturn(void){
    thread_t *curr;
    GETPRO(curr);

    do_sys_sigreturn((void*) (curr->context));

    return 0;
}

/**
 * @brief syscall for set specific signal handler to the Current thread
 * 
 * @return return 0 if successfull
*/
asmlinkage int32_t sys_set_handler(int32_t signum, void *handler_addr){
    thread_t *curr;
    GETPRO(curr);

    if (signum >= SIG_COUNT || signum < 0) return -1;
   
    int flags;
    cli_and_save(flags);
    if (handler_addr != NULL) {
        curr->sig->exe_sig_act[signum] = (default_action) handler_addr;
    } else {
        curr->sig->exe_sig_act[signum] = default_arr[signum];
    }
    restore_flags(flags);

    return 0;
}

/**
 * @brief add the mask of corrosponding signal
 * 
 * 
*/
int32_t add_mask(thread_t *thread, int signum){
    if (signum >= SIG_COUNT || signum < 0) return -1;

    int flags;
    cli_and_save(flags);

    thread->sig->mask_arr[signum] = 1;

    restore_flags(flags);

    return 0;
}

/**
 * @brief remove the mask of corrosponding signal
 * 
 * 
*/
int32_t remove_mask(thread_t *thread, int signum){
    if (signum >= SIG_COUNT || signum < 0) return -1;
    int flags;
    cli_and_save(flags);

    thread->sig->mask_arr[signum] = 0;
    
    restore_flags(flags);
    return 0;
}

/**
 * @brief when signal send to the process, record the signal 
 * 
 * 
*/
int32_t send_signal(thread_t *thread, int8_t signum){
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


/**
 * @brief initialize the default handler 
 * 
 * 
*/
int32_t sig_init(void){
    default_arr[DIV_ZERO] = &div_zero_hdler;
    default_arr[SEGFAULT] = &div_zero_hdler;
    default_arr[INTERRUPT] = &interrupt_hdler;
    default_arr[ALARM] = &alarm_hdler;
    default_arr[USER1] = &user1_hdler;
    
    return 0;
}

/** 
 * @brief occurs when Kernel ----> Usr, deliver pending signal
 *   
 * @return return 0 indicates succeed, otherwise -1
*/
int32_t deliver_signal(thread_t *thread){
    /* First mask all signal and store the original stuff */ 
    int32_t i, flags, sig_num;

    cli_and_save(flags);



    /* store the previous mask state and then mask all other signals */
    for (i = 0; i < SIG_COUNT; ++ i) {
        thread->sig->previous_mask_arr[i] = thread->sig->mask_arr[i];
        add_mask(thread, i);
    }

    /* mask all signal*/
    for (i = 0; i < SIG_COUNT; ++ i) {
        if (thread->sig->mask_arr[i] != 1 && thread->sig->pen_arr[i] == 1) {
            sig_num = i;
            break;
        }
    }

    sig_num = 1;  /* need to commnets out *////////////////////////////////////////////////////////

    /* According to ULK P.441, if ka.sa.sa_handler is equal to ISG_DFL, we must perform default handler */
    if (thread->sig->exe_sig_act[sig_num] == default_arr[sig_num]) {
        default_arr[sig_num]();    
    } else {
        do_deliver((void*) (thread->context), thread->sig->exe_sig_act[sig_num], sig_num);
    }

    /*unmask and change previous_mask to the initial state */
    for (i = 0; i < SIG_COUNT; ++ i) {
        thread->sig->mask_arr[i] = thread->sig->previous_mask_arr[i];
        thread->sig->previous_mask_arr[i] = 0;
    }

    restore_flags(flags);
    return 0;
}

/**
 * @brief initialize the intial state of signal_struct_t of each thread 
 * 
 * 
*/
int32_t thread_sig_init(thread_t *thread){
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


