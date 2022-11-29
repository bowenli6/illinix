#include <boot/signal.h>

.text

.global do_deliver sys_sig_return

sig_return:
    int $0x80

# CURRENT->context, CURRENT->sig->exe_sig_act[sig_num], sig_num
do_deliver:
    pushl %ebp
    movl  %esp, %ebp

    pushl %eax
    pushl %ebx
    pushl %ecx 
    pushl %edx 
    pushl %esi 
    pushl %edi

    movl %esp, %eax                 # eax = kernel_esp
    movl %ebp, %ebx                 # ebx = kernel_ebp
    movl 8(%ebp), %ecx              # ecx = sig_num
    movl 12(%ebp), %edx             # edx = *hw_context
    movl 16(%ebp), %esi             # esi = handler


    movl    12(%ebp), %edx
    movl    60(%edx), %esp         # esp = user_esp
    movl    20(%edx), %ebp         # ebp = user_ebp

    movl   %esp, %edi               # edi = curr_usr_esp
    # put the handler
    pushl   sig_return

    # put the hw context
    pushl   64(%edx)
    pushl   60(%edx)
    pushl   56(%edx)
    pushl   52(%edx)
    pushl   48(%edx)
    pushl   44(%edx)
    pushl   40(%edx)
    pushl   36(%edx)
    pushl   32(%edx)
    pushl   28(%edx)
    pushl   24(%edx)
    pushl   20(%edx)
    pushl   16(%edx)
    pushl   12(%edx)
    pushl   8(%edx)
    pushl   4(%edx)
    pushl   0(%edx)

    # put the sig_num
    pushl  %ecx

    # put the return addr
    pushl  %edi
    
    # transfer the control to uesr mode
    # especially the value of eip
    movl %esp, 60(%edx)
    movl %esi, %eip


    # when gets back from usr, reset the kernel esp and kernel ebp
    movl 

    popl %edi
    popl %esi
    popl %edx 
    popl %ecx 
    popl %ebx 
    popl %eax

    leav 
    ret 

do_syssig_return:
