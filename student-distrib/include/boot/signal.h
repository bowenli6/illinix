#ifndef _SIGNAL_H
#define _SIGNAL_H

#include <boot/syscall.h>
#include <pro/process.h>

#define SIG_COUNT   5   /* There are up to five kinds of signal */

#define DIV_ZERO    0
#define SEGFAULT    1
#define INTERRUPT   2
#define ALARM       3
#define USER1       4
                                                    
/* Actions performed upon delivering a signal */
#define ACT_COUNT       7
#define ACT_EXP_IGNORE  0
#define ACT_TERMINATE   1
#define ACT_DUMP        2
#define ACT_IGNORE      3        
#define ACT_STOP        4
#define ACT_CONTINUE    5
#define ACT_CATCH       6

#define get_sig
        
typedef int32_t (*default_action)(void);

typedef struct signal {
    int8_t pending_sig_count;
    int8_t curr_sig;                                    /* signal index the thread is using; -1 means we are not using any */
    int8_t mask_arr[SIG_COUNT];                         /* 0 means not mask, 1 means mask */
    int8_t pen_arr[SIG_COUNT];                          /* 0 means not pending, 1 means pending */
    int32_t *exe_sig_act[SIG_COUNT];                    /* use signal as index, and corrosponding value indicates the handler */
} signal_struct_t;  

default_action default_arr[SIG_COUNT];

extern int32_t sig_init();

extern int32_t thread_sig_init(thread_t* thread);

extern int32_t send_signal(thread_t* thread, int8_t signum);

asmlinkage extern int32_t deliver_signal();

#endif 
