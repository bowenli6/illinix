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
    int8_t          pending_sig_count;
    int8_t          curr_sig;                                    /* signal index the thread is using; -1 means we are not using any */
    int8_t          mask_arr[SIG_COUNT];                         /* 0 means not mask, 1 means mask */
    int8_t          previous_mask_arr[SIG_COUNT];                /* previous mask, we need to store them before */
    int8_t          pen_arr[SIG_COUNT];                          /* 0 means not pending, 1 means pending */
    default_action  exe_sig_act[SIG_COUNT];                    /* use signal as index, and corrosponding value indicates the handler */
} signal_struct_t;  

typedef struct hardware_context{
    int32_t EBX;
    int32_t ECX;
    int32_t EDX;
    int32_t ESI;
    int32_t EDI;
    int32_t EBP;
    int32_t EAX;
    int16_t DS;
    int16_t ES;
    int16_t FS;
    int32_t IRQ;
    int32_t ERROR_CODE;
    int32_t RETURN_ADDR;
    int16_t CS;
    int32_t EFLAGS;
    int32_t ESP;
    int16_t SS;    
} hardware_context_t;

default_action default_arr[SIG_COUNT];

extern int32_t sig_init();

extern int32_t thread_sig_init(thread_t* thread);

extern int32_t send_signal(thread_t* thread, int8_t signum);

asmlinkage extern int32_t deliver_signal();

#endif 
