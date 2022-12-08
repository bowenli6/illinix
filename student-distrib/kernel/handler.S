
.data

# Constants for accessing the fields of the stack
EAX      = 0x18
ES       = 0x20
ORIG_EAX = 0x24
EIP      = 0x30
INTR     = 0x24
NCALL    = 21
USER_DS  = 0x002B

syscall_table:
    .long sys_restart   /* Used for restarting */
	.long sys_exit
	.long sys_execute
	.long sys_read
	.long sys_write
	.long sys_open
    .long sys_close
    .long sys_getargs
    .long sys_vidmap
    .long sys_set_handler
    .long sys_sigreturn
    .long sys_fork
    .long sys_execv 
    .long sys_wait
    .long sys_waitpid 
    .long sys_getpid
    .long sys_getppid 
    .long sys_sbrk
    .long sys_mmap
    .long sys_munmap
    .long sys_stat
.text

# Save all the CPU registers that may be used by the exception handler on the stack.
#define SAVE_ALL \
	cld; \
	pushl %es; \
	pushl %ds; \
	pushl %eax; \
	pushl %ebp; \
	pushl %edi; \
	pushl %esi; \
	pushl %edx; \
	pushl %ecx; \
	pushl %ebx; \
	movl $USER_DS, %edx; \
	movl %edx, %ds; \
	movl %edx, %es;


#define RESTORE_INT_REGS \
	popl %ebx;	\
	popl %ecx;	\
	popl %edx;	\
	popl %esi;	\
	popl %edi;	\
	popl %ebp;	\
	popl %eax


#define RESTORE_ALL	\
    RESTORE_INT_REGS
    popl %ds;	    \
	popl %es;	    \
	addl $4, %esp;	\
	iret;		    

error_code:
    # Save the registers that might be used by the do_handler.
    pushl   %ds
	pushl   %eax
	pushl   %ebp
	pushl   %edi
	pushl   %esi
	pushl   %edx
	pushl   %ecx
	pushl   %ebx
    xorl    %eax, %eax      # eax = 0
	decl    %eax			# eax = -1

    # Clear the direction flag DF of eflags, 
    # making sure that autoincreases on the
    # edi and esi registers will be used with 
    # string instructions.
	cld                     
    
	movl    ES(%esp), %edi		    # get the handler function address: esp+32
	movl    ORIG_EAX(%esp), %edx	# get the hardware error code: esp+36

    # Store -1 to the location for error code.
    # This value is used to separate 0x80 excetion
    # (system calls) from other exception.
    movl    %eax, ORIG_EAX(%esp)   

    movl    %es, %ecx               # save one extra segment determined by the programmer (ES)  
    movl    %ecx, ES(%esp)          # write ES in the stack location.

    # TODO does not loads the user data segment selector into ds and es registers.

    # The invoked function receives its arguments from the eax and edx registers
    # rather using the Standard C calling convention. The current version of 
    # exception handlers dose not receive arguments

    # disable intrrupts
    cli
    call    *%edi                   # Invokes the do_handler function.
    sti 
	jmp     ret_from_exception      # returns from the exception.


ret_from_exception:
    jmp     abort

abort:
    pushl   $256
    call    do_exit

# Exception handlers

.globl divide_error_handler
divide_error_handler:
    pushl   $0                  # no error code now: pad the hardware error code on the stack.
    pushl   $do_divide_error    # push the do_handler function address. 
    jmp     error_code          # part of theses are only useful when syscall is working.

.globl debug_handler
debug_handler:
    pushl   $0                  # no error code now: pad the hardware error code on the stack.
    pushl   $do_debug           # push the do_handler function address. 
    jmp     error_code          # part of theses are only useful when syscall is working.

.globl nmi_handler
nmi_handler:
    pushl   $0                  # no error code now: pad the hardware error code on the stack.
    pushl   $do_nmi             # push the do_handler function address. 
    jmp     error_code          # part of theses are only useful when syscall is working.

.globl int3_handler
int3_handler:
    pushl   $0                  # no error code now: pad the hardware error code on the stack.
    pushl   $do_int3            # push the do_handler function address. 
    jmp     error_code          # part of theses are only useful when syscall is working.

.globl overflow_handler
overflow_handler:
    pushl   $0                  # no error code now: pad the hardware error code on the stack.
    pushl   $do_overflow        # push the do_handler function address. 
    jmp     error_code          # part of theses are only useful when syscall is working.

.globl bounds_handler
bounds_handler:
    pushl   $0                  # no error code now: pad the hardware error code on the stack.
    pushl   $do_bounds          # push the do_handler function address. 
    jmp     error_code          # part of theses are only useful when syscall is working.

.globl invalid_op_handler
invalid_op_handler:
    pushl   $0                  # no error code now: pad the hardware error code on the stack.
    pushl   $do_invalid_op      # push the do_handler function address. 
    jmp     error_code          # part of theses are only useful when syscall is working.

.globl device_not_available_handler
device_not_available_handler:
    pushl   $0                      # no error code now: pad the hardware error code on the stack.
    pushl   $do_device_not_available# push the do_handler function address. 
    jmp     error_code              # part of theses are only useful when syscall is working.

.globl double_fault_handler
double_fault_handler:
    pushl   $0                  # no error code now: pad the hardware error code on the stack.
    pushl   $do_double_fault    # push the do_handler function address. 
    jmp     error_code          # part of theses are only useful when syscall is working.

.globl coprocessor_segment_overrun_handler
coprocessor_segment_overrun_handler:
    pushl   $0                              # no error code now: pad the hardware error code on the stack.
    pushl   $do_coprocessor_segment_overrun # push the do_handler function address. 
    jmp     error_code                      # part of theses are only useful when syscall is working.

.globl invalid_TSS_handler
invalid_TSS_handler:
    pushl   $0                  # no error code now: pad the hardware error code on the stack.
    pushl   $do_invalid_TSS     # push the do_handler function address. 
    jmp     error_code          # part of theses are only useful when syscall is working.


.globl segment_not_present_handler
segment_not_present_handler:
    pushl   $0                          # no error code now: pad the hardware error code on the stack.
    pushl   $do_segment_not_present     # push the do_handler function address. 
    jmp     error_code                  # part of theses are only useful when syscall is working.


.globl stack_segment_handler
stack_segment_handler:
    pushl   $0                  # no error code now: pad the hardware error code on the stack.
    pushl   $do_stack_segment   # push the do_handler function address. 
    jmp     error_code          # part of theses are only useful when syscall is working.


.globl general_protection_handler
general_protection_handler:
    pushl   $0                          # no error code now: pad the hardware error code on the stack.
    pushl   $do_general_protection      # push the do_handler function address. 
    jmp     error_code                  # part of theses are only useful when syscall is working.


.globl page_fault_handler
page_fault_handler:
    pushal
    movl	%cr2, %eax
	pushl	%eax		
    pushl   $0                  # no error code now: pad the hardware error code on the stack.
    # pushl   $do_page_fault      # push the do_handler function address. 
    call    do_page_fault
    add     $12, %esp
    popal
    # movl    $USER_DS, %eax
    # andl    $0xFF, %eax
    # movw	%ax, %ds
    # pushl   %eax
    # movl	$, dest
    
    # pushl   %ebx
    

    #jmp     error_code          # part of theses are only useful when syscall is working.
    iret


.globl coprocessor_error_handler
coprocessor_error_handler:
    pushl   $0                      # no error code now: pad the hardware error code on the stack.
    pushl   $do_coprocessor_error   # push the do_handler function address. 
    jmp     error_code              # part of theses are only useful when syscall is working.


.globl alignment_check_handler
alignment_check_handler:
    pushl   $0                      # no error code now: pad the hardware error code on the stack.
    pushl   $do_alignment_check     # push the do_handler function address. 
    jmp     error_code              # part of theses are only useful when syscall is working.

.globl machine_check_handler
machine_check_handler:
    pushl   $0                      # no error code now: pad the hardware error code on the stack.
    pushl   $do_machine_check       # push the do_handler function address. 
    jmp     error_code              # part of theses are only useful when syscall is working.

.globl simd_coprocessor_error_handler
simd_coprocessor_error_handler:
    pushl   $0                              # no error code now: pad the hardware error code on the stack.
    pushl   $do_simd_coprocessor_error      # push the do_handler function address. 
    jmp     error_code                      # part of theses are only useful when syscall is working.




# Interrupt handlers


.globl timer_handler
timer_handler:
    pushfl  
    pushal
    call    do_timer
    popal
    popfl
    iret

.globl keyboard_handler
keyboard_handler:
    pushfl  
    pushal
    call    do_keyboard
    popal
    popfl
    iret



.globl rtc_handler
rtc_handler:
    pushfl  
    pushal
    call    do_rtc
    popal
    popfl
    iret



# System calls linkage
.globl syscall_handler
syscall_handler:
    pushl %eax                              # save the system call number.
	pushl %es
	pushl %ds
	pushl %eax
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx
    cmpl  $NCALL, %eax                      # validity check performed on the system call number.
    jb    nobadsys
    movl  $-1, EAX(%esp)                    # set the error number.
    jmp   resume_userspace                  # jump back to user mode.
nobadsys:
    call  *syscall_table(, %eax, 4)         # perform the system call.
    movl  %eax, EAX(%esp)		            # store the return value
syscall_exit:
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    popl %eax
    popl %ds
    popl %es
    addl $4, %esp
    iret;		    

resume_userspace:
    # TODO: Check for recheduling request, not implmented yet.
    jmp   syscall_exit

