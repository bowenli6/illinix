/* Call the main() function, then halt with its return value. */

.globl syscall
syscall:
	pushl	%ebx
	movl	8(%esp),  %eax
	movl	12(%esp), %ebx
	movl	16(%esp), %ecx
	movl	20(%esp), %edx
	int		$0x80
	popl	%ebx
	ret


.globl _start
_start:
	CALL	main
    PUSHL   $0
    PUSHL   $0
	PUSHL	%EAX
	CALL	_exit

