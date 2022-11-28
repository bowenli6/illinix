
.data 

# Constants for accessing the fields of the context_t
EAX = 0
EBX = 4
ECX = 8
EDX = 12
ESI = 16
EDI = 20
EBP = 24
ESP = 28
EFLAGS = 32
ES = 36
CS = 40
DS = 44
FS = 48
GS = 52

.text

# arg0: the hardware context of the thread that to be saved
.globl save_context
save_context:
    pushl   %ebp;
    movl    %esp, %ebp;

    pushl   %eax

    # get argument
    movl    8(%ebp), %eax 

    # save edx
    pushl   %edx 

    movl    4(%esp), %edx 

    # store current hardware context to eax
    movl    %edx,    EAX(%eax)

    popl    %edx

    movl    %edx,    EDX(%eax)
    movl    %ebx,    EBX(%eax)
    movl    %esi,    ESI(%eax)
    movl    %edi,    EDI(%eax)
    movl    %ebp,    EBP(%eax)
    movl    %esp,    ESP(%eax)
    pushfl
    popl    %ecx
    movl    %ecx,    EFLAGS(%eax)
    movw    %es,     ES(%eax)
    movw    %cs,     CS(%eax)
    movw    %ds,     DS(%eax)
    movw    %fs,     FS(%eax)
    movw    %gs,     GS(%eax)

    leave 
    ret


# arg0: the hardware context of the thread that wants to switch
# arg0: the hardware context of the thread that will be switched with
.globl swtch
swtch:
    pushl   %ebp;
    movl    %esp, %ebp;

    # save eax 
    pushl   %eax

    # get first argument
    movl    8(%ebp), %eax 
    
    testl   %eax, %eax  # arg0 is NULL
    je      to

    # save edx
    pushl   %edx 

    movl    4(%esp), %edx 

    # store current hardware context to eax
    movl    %edx,    EAX(%eax)

    popl    %edx

    movl    %edx,    EDX(%eax)
    movl    %ebx,    EBX(%eax)
    movl    %esi,    ESI(%eax)
    movl    %edi,    EDI(%eax)
    movl    %ebp,    EBP(%eax)
    movl    %esp,    ESP(%eax)
    pushfl
    popl    %ecx
    movl    %ecx,    EFLAGS(%eax)
    movw    %es,     ES(%eax)
    movw    %cs,     CS(%eax)
    movw    %ds,     DS(%eax)
    movw    %fs,     FS(%eax)
    movw    %gs,     GS(%eax)

to: 
    # get second argument
    movl    12(%ebp), %edx 

    # load edx to the current hardware context 
    movl    EAX(%edx), %eax
    movl    EBX(%edx), %ebx
    movl    ESI(%edx), %esi
    movl    EDI(%edx), %edi
    movl    EBP(%edx), %ebp
    movl    ESP(%edx), %esp
    pushl   EFLAGS(%edx)
    popfl
    movw    ES(%edx),  %es
    movw    CS(%edx),  %cs
    movw    DS(%edx),  %ds
    movw    FS(%edx),  %fs
    movw    GS(%edx),  %gs
    movl    EDX(%edx), %edx

    leave
    ret 

