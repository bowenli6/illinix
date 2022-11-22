#ifndef _SIGNAL_H
#define _SIGNAL_H

#include <boot/syscall.h>

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
        
typedef int32_t (*default_action)(void);

typedef struct signal {
    uint8_t curr_sig;                                                           /* signal index we are using */
    uint8_t mask_arr[SIG_COUNT];                                                 /* use signal as index, and corrosponding value indicates whether mask or not */
    uint8_t pen_arr[SIG_COUNT];
    int32_t *exe_sig_act[SIG_COUNT];                                            /* use signal as index, and corrosponding value indicates the handler */
} signal_struct_t;  

default_action default_arr[SIG_COUNT];

extern int32_t sig_init();


#endif 
