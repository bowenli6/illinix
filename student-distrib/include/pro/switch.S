#include <pro/process.h>

.data 

# Constants for accessing the fields of the conext_t
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
    pushl   %edx 

    # get arguments
    movl    8(%ebp), %eax 

    # store current hardware context to eax
    movl    4(%esp), EAX(%eax)
    movl    0(%esp), EBX(%eax)
    movl    %edx,    EDX(%eax)
    movl    %esi,    ESI(%eax)
    movl    %edi,    EDI(%eax)
    movl    %ebp,    EBP(%eax)
    movl    %esp,    ESP(%eax)
    pushfl
    popl    %ecx
    movl    %ecx,    EFLAGS(%eax)
    movl    %es,     ES(%eax)
    movl    %cs,     CS(%eax)
    movl    %ds,     DS(%eax)
    movl    %fs,     FS(%eax)
    movl    %gs,     GS(%eax)

    leave 
    ret


# arg0: the hardware context of the thread that wants to switch
# arg0: the hardware context of the thread that will be switched with
.globl context_switch
context_switch:
    pushl   %ebp;
    movl    %esp, %ebp;

    pushl   %eax
    pushl   %edx 

    # get arguments
    movl    8(%ebp), %eax 
    movl    12(%ebp), %edx

    # store current hardware context to eax
    movl    4(%esp), EAX(%eax)
    movl    0(%esp), EBX(%eax)
    movl    %edx,    EDX(%eax)
    movl    %esi,    ESI(%eax)
    movl    %edi,    EDI(%eax)
    movl    %ebp,    EBP(%eax)
    movl    %esp,    ESP(%eax)
    pushfl
    popl    %ecx
    movl    %ecx,    EFLAGS(%eax)
    movl    %es,     ES(%eax)
    movl    %cs,     CS(%eax)
    movl    %ds,     DS(%eax)
    movl    %fs,     FS(%eax)
    movl    %gs,     GS(%eax)

    # load edx to the current hardware context 
    movl    EAX(%edx), %eax
    movl    EBX(%edx), %ebx
    movl    EDX(%edx), %edx
    movl    ESI(%edx), %esi
    movl    EDI(%edx), %edi
    movl    EBP(%edx), %ebp
    movl    ESP(%edx), %esp
    pushl   EFLAGS(%edx)
    popfl
    movl    ES(%edx),  %es
    movl    CS(%edx),  %cs
    movl    DS(%edx),  %ds
    movl    FS(%edx),  %fs
    movl    GS(%edx),  %gs

    leave
    ret 

