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
retcode: 
    .long 0
pretcode:
    .long 0

usr_esp_addr:
    .long 0
ret_kernel_esp:
    .long 0
ret_kernel_ebp:
    .long 0
ret_usr_esp_addr:
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
    movl 8(%ebp), %ebx             # ebx = address of usr_esp in process_t
    movl 12(%ebp), handler
    movl 16(%ebp), sig_num 
    movl %ebx, usr_esp_addr

    # save the user register  
    movl (%ebx), user_esp
    movl 4(%ebx), user_ebp
    movl 8(%ebx), user_eip  

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
    pushl user_esp
    pushl user_ebp
    pushl user_eip 


    # put the sinal numer
    pushl sig_num

    # put the preturn code for the linkage
    pushl pretcode

    # update the CURRETN->esp
    movl usr_esp_addr, %ebx
    movl %esp, (%ebx)

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


# void *reg
.globl do_sys_sigreturn
do_sys_sigreturn:
      
    pushl %ebp
    movl  %esp, %ebp
    pushl %edi
    pushl %esi
    pushl %ebx

    # get usr edi = addr of CURRENT->ESP
    movl 8(%ebp), %edi 
    movl 8(%ebp), ret_usr_esp_addr
    movl (%edi), %edi           # edi = CURRENT -> ESP
    movl ret_usr_esp_addr, %esi

    # change CURRENT->eip/esp/ebp
    movl 8(%edi), %ebx
    movl %ebx, 8(%esi)
    movl 12(%edi), %ebx
    movl %ebx, 4(%esi)
    movl 16(%edi), %ebx
    movl %ebx, 0(%esi)  

    popl %ebx 
    popl %esi
    popl %edi
    
    leav 
    ret  
