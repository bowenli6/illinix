
# calling rountine should be: signal_handler(user_space) -> 
#                             linkage_for_sig_return -> syscal_handler -> sys_ig_return -> user_space
# for store regiser, and argument value 
.data

EIP = 0
ESP = 4
EBX = 8
ECX = 12
EDX = 16
ESI = 20
EDI = 24
EBP = 28
EAX = 32

BACK_EAX = 8
BACK_EBP = 12
BACK_EDI = 16
BACK_ESI = 20
BACK_EDX = 24
BACK_ECX = 28
BACK_EBX = 32
BACK_ESP = 36
BACK_EIP = 40



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
retcode: 
    .long 0
pretcode:
    .long 0

usr_context_addr:
    .long 0
ret_kernel_esp:
    .long 0
ret_kernel_ebp:
    .long 0
ret_usr_context_addr:
    .long

.text
linkage_for_sigreturn:
    movl    $10, %eax          # value for sys_sigreturn is 10
    int     $0x80

# this part of code will setup the frame
# do_deliver(void *reg, default_action func, int32_t num);
.globl do_deliver 
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
    movl 8(%ebp), %ebx             # ebx = address of usr_context_addr in process_t
    movl 12(%ebp), %esi
    movl %esi, handler
    movl 16(%ebp), %esi 
    movl %esi, sig_num

    # get thread->context pointer 
    movl %ebx, usr_context_addr

    # save the user register  
    movl EIP(%ebx), %esi
    movl %esi, user_eip   
    movl ESP(%ebx), %esi 
    movl %esi, user_esp 
    movl EBP(%ebx), %esi 
    movl %esi, user_ebp 

    # change to user stack
    movl user_esp, %esp
    movl user_ebp, %ebp 

    # set up the return code
    movl linkage_for_sigreturn, %edi
    movl linkage_for_sigreturn + 3, %esi

    subl $7, %esp
    movl %edi, (%esp)
    movl %esi, 3(%esp)

    movl %esp, pretcode
    subl $1, %esp 

    # put reg on user stack
    movl usr_context_addr, %ebx  
    pushl user_eip                  # eip reg
    pushl user_esp                  # esp reg 
    movl  EBX(%ebx), %esi
    pushl %esi                      # ebx reg  
    movl  ECX(%ebx), %esi           
    pushl %esi                      # ecx reg
    movl  EDX(%ebx), %esi
    pushl %esi                      # edx reg 
    movl  ESI(%ebx), %esi
    pushl %esi                      # esi reg
    movl  EDI(%ebx), %esi
    pushl %esi                      # edi reg 
    pushl user_ebp                  # ebp reg 
    movl  EAX(%ebx), %esi
    pushl %esi                      # eax reg 

    # put the sinal numer
    pushl sig_num

    # put the preturn code for the linkage
    pushl pretcode

    # update the current esp valueS
    movl usr_context_addr, %ebx
    movl %esp, ESP(%ebx)

    # change the eipin context
    movl usr_context_addr, %ebx
    movl handler, %esi 
    movl %esi, EIP(%ebx) 

    movl kernel_esp, %esp
    movl kernel_ebp, %ebp 
    
    popl %edi
    popl %esi
    popl %ebx

    leave
    ret 


# void *context
.globl do_sys_sigreturn
do_sys_sigreturn:
      
    pushl %ebp
    movl  %esp, %ebp
    pushl %edi
    pushl %esi
    pushl %ebx

    # get addr of thread context
    movl 8(%ebp), %edi 
    movl %edi, ret_usr_context_addr
    movl ESP(%edi), %edi                       # edi = context -> ESP
    movl ret_usr_context_addr, %esi

    # change CURRENT->eip/esp/ebp
    movl BACK_EAX(%edi), %ebx
    movl %ebx, EAX(%esi)
    movl BACK_EBP(%edi), %ebx
    movl %ebx, EBP(%esi)
    movl BACK_EDI(%edi), %ebx
    movl %ebx, EDI(%esi)
    movl BACK_ESI(%edi), %ebx
    movl %ebx, ESI(%esi)
    movl BACK_EDX(%edi), %ebx
    movl %ebx, EDX(%esi)
    movl BACK_ECX(%edi), %ebx
    movl %ebx, ECX(%esi)
    movl BACK_EBX(%edi), %ebx
    movl %ebx, EBX(%esi)
    movl BACK_ESP(%edi), %ebx
    movl %ebx, ESP(%esi)
    movl BACK_EIP(%edi), %ebx
    movl %ebx, EIP(%esi)
    

    popl %ebx 
    popl %esi
    popl %edi
    
    leave
    ret  
