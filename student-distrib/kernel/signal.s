#include <boot/signal.h>

.text

.global do_deliver 
.global do_syssig_return
kernel_hw_context_ptr: .long 0
user_hw_context_ptr: .long 0


linkage_for_sigreturn:
    movl    $10, %eax          # value for sys_sigreturn is 10
    int     $0x80

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

    movl %esp, %edi                 # edi = kernel_esp
    movl %ebp, %esi                 # esi = kernel_ebp
    movl 8(%ebp), %ecx              # ecx = sig_num
    movl 12(%ebp), %edx             # edx = *hw_context
    movl 16(%ebp), %eax             # eax = handler


    movl    60(%edx), %esp         # esp = user_esp
    movl    20(%edx), %ebp         # ebp = user_ebp
    movl    %esp, %ebx              # ebx = curr_usr_esp

    # put the assembly linkage on the stack
    pushl   linkage_for_sigreturn

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
    # put the return addr which is curr_usr_esp
    pushl  %ebx
    
    # update esp of hw context and eip of hw context
    movl %esp, 60(%edx)
    movl %eax, %eip

    # esp = kernel_esp, ebp = kernel_ebp
    movl %edi, %esp
    movl %esi, %ebp

    popl %edi
    popl %esi
    popl %edx 
    popl %ecx 
    popl %ebx 
    popl %eax

    leav 
    ret 

do_syssig_return:

      movl 64(%esp), %edi
      addl $4, %edi
      movl %esp, %esi
      addl $4, %esi

      movl %esi, kernel_hw_context_ptr
      movl %edi, user_hw_context_ptr


      xorl %ebx, %ebx

copy_loop:

      movl  (%edi,%ebx,4), %ecx
      movl  %ecx, (%esi,%ebx,4)
      addl  $1,  %ebx
      cmpl  $17, %ebx
      jne   copy_loop


      pushl %edx
      pushl %ecx
      pushl %eax

      call restore_signal

      popl  %eax
      popl  %ecx
      popl  %edx

      movl kernel_hw_context_ptr, %esi

      movl 24(%esi), %eax

      ret