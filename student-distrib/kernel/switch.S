
.data 

# Constants for accessing the fields of the context_t
EAX = 0
ECX = 4
EDX = 8
EBP = 12
ESP = 16
EIP = 20
EFLAGS = 24
ES = 28
FS = 32
GS = 36

.text

# arg0: the hardware context of the thread that wants to switch
# arg1: the hardware context of the thread that will be switched with
.globl __swtch
__swtch:
    pushl   %ebx

    # get first argument
    movl    4(%esp), %ebx 
    
    # store current hardware context to eax
    movl    %eax,    EAX(%ebx)
    movl    %edx,    EDX(%ebx)
    movl    %ecx,    ECX(%ebx)
    pushfl
    popl    %eax
    movl    %eax,    EFLAGS(%ebx)
    movw    %es,     ES(%ebx)
    movw    %fs,     FS(%ebx)
    movw    %gs,     GS(%ebx)

    # get second argument
    movl    8(%esp), %ebx 

    # load edx to the current hardware context 
    movl    EAX(%ebx), %eax
    movl    EDX(%ebx), %edx
    movl    ECX(%ebx), %ecx
    pushl   EFLAGS(%ebx)
    popfl
    movw    ES(%ebx),  %es
    movw    FS(%ebx),  %fs
    movw    GS(%ebx),  %gs
   
    popl    %ebx
    addl    $8, %esp 

    ret 

