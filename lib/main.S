/* Call the main() function, then halt with its return value. */

.GLOBAL _start
_start:
	CALL	main
    PUSHL   $0
    PUSHL   $0
	PUSHL	%EAX
	CALL	exit

