#include "tests.h"
#include "../x86_desc.h"
#include "../lib/lib.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/**
 * @brief that first 10 IDT entries are not NULL
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test() {
	TEST_HEADER;
	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}
	return result;
}


/**
 * @brief Observe if the exception handler 
 * prints the error message.
 */
void divide_error() {
	TEST_HEADER;
	int one = 1;
	int zero = 0;
	int check = one / zero;
	printf("I don't like warning, so I don't care what %d is\n", check);
}


/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests() {
	printf("--------------------------- Test begins ---------------------------\n");
	TEST_OUTPUT("idt_test", idt_test());
	divide_error();
	printf("---------------------------- Test Ends ----------------------------\n");

}
