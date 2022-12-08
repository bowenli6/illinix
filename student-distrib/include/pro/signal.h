#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include <pro/process.h>
#include <boot/syscall.h>

#define DIV_ZERO    0
#define SEGFAULT    1
#define INTERRUPT   2
#define ALARM       3
#define USER1       4
                                                    
int32_t send_signal(thread_t *thread, int8_t signum);
int32_t add_mask(thread_t *thread, int signum);
int32_t remove_mask(thread_t *thread, int signum);
int32_t sig_init(void);
int32_t thread_sig_init(thread_t *thread);
int32_t deliver_signal(thread_t *thread);
void do_sys_sigreturn(void *reg);
void do_deliver(void *reg, default_action func, int32_t num);


#endif /* SIGNAL_H */

