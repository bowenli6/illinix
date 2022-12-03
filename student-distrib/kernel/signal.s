#include <boot/signal.h>

# for store regiser, and argument value 
.data

user_esp: 
    .long 0
user_ebp: 
    .long 0
user_eip: 
    .long 0
kernel_esp: 
    .long 0
kernel_ebp: 
    .long 0
kernel_eip: 
    .long 0
handler: 
    .long 0
sig_num: 
    .long 0
return_addr: 
    .long 0


# need to delete 
kernel_hw_context_ptr: .long 0
user_hw_context_ptr: .long 0

.text
.global do_deliver 
.global sys_sigreturn

linkage_for_sigreturn:
    movl    $10, %eax          # value for sys_sigreturn is 10
    int     $0x80

# this part of code will setup the frame
do_deliver:
    pushl %ebp
    movl  %esp, %ebp

    pushl %ebx 
    pushl %esi
    pushl %edi 

    # save the kernel esp and kernel ebp
    movl %esp, kernel_esp
    movl %ebp, kernel_ebp

    # get value from argument
    movl 8(%ebp), %ebx             # ebx = address of usr_esp in process_t
    movl 12(%ebp), handler
    movl 16(%ebp), sig_num 

    # save the user register  
    movl (%ebx), user_esp
    movl 4(%ebx), user_ebp
    movl 8(%ebx), user_eip  

    # change to user stack
    movl %user_esp, %esp
    movl %user_ebp, %ebp 

    # put reg on user stack
    pushl user_ebp
    pushl user_eip 
    pushl user_esp

    # put the sinal numer
    pushl sig_num

    # put the eip of the linkage
    movl linkage_for_sigreturn, %ebx
    pushl %ebx

    # call the handler
    movl handler, %eip
    # when the handler return, it will resume by calling linkage
    # linkage will do int $80 and call sys_sigreturn

    movl kernel_esp, %esp
    movl kernel_ebp, %ebp 
    
    popl %edi
    popl %esi
    popl %ebx

    leav 
    ret 

sys_sigreturn:

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