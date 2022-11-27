#include <boot/signal.h>

.text

.global do_deliver

# CURRENT->context, CURRENT->sig->exe_sig_act[sig_num], sig_num
do_deliver:
    pushl %esp
    movl  
