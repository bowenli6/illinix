#include <sysnum.h>


#define DO_CALL(name,number)   \
.GLOBL name                   ;\
name:   PUSHL	%EBX          ;\
	MOVL	$number,%EAX  ;\
	MOVL	8(%ESP),%EBX  ;\
	MOVL	12(%ESP),%ECX ;\
	MOVL	16(%ESP),%EDX ;\
	INT	$0x80         ;\
	POPL	%EBX          ;\
	RET


/* Call the main() function, then halt with its return value. */

.GLOBAL _start
_start:
	CALL	main
    PUSHL   $0
    PUSHL   $0
	PUSHL	%EAX
	CALL	exit

