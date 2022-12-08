
.data 

# Constants for accessing the fields of the context_t
EIP = 0
ESP = 4
EBX = 8
ECX = 12
EDX = 16
ESI = 20
EDI = 24
EBP = 28
EAX = 32


.text

# prev: the hardware context of the thread that wants to switch
# next: the hardware context of the thread that will be switched with
.globl swtch
swtch:
    # get first argument
    movl   4(%esp), %eax 

    # save current context into prev context
    popl   EIP(%eax)
    movl   %esp, ESP(%eax)
    movl   %ebx, EBX(%eax)
    movl   %ecx, ECX(%eax)
    movl   %edx, EDX(%eax)
    movl   %esi, ESI(%eax)
    movl   %edi, EDI(%eax)
    movl   %ebp, EBP(%eax)

    #get second argument 
    movl   4(%esp), %eax

    # restore next context into current context
    movl   EBP(%eax), %ebp
    movl   EDI(%eax), %edi
    movl   ESI(%eax), %esi
    movl   EDX(%eax), %edx
    movl   ECX(%eax), %ecx
    movl   EBX(%eax), %ebx
    movl   ESP(%eax), %esp

    # set new return address
    pushl  EIP(%eax)

    # return value 
    movl   EAX(%eax), %eax

    ret

