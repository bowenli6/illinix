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
 * 
 */
void divide_error() {
	TEST_HEADER;
	clear();
	int one = 1;
	int zero = 0;
	int check = one / zero;
	printf("I don't like warning, so I don't care what %d is\n", check);
}


/**
 * @brief Observe if the syscall handler
 * print the massage.
 * 
 */
void inline syscall_check() {
	TEST_HEADER;
	clear();
	asm volatile("int $128");
	asm volatile("int $128");
	asm volatile("int $128");
}

/**
 * @brief Observe if status of PDEs and PTEs 
 * are correct.
 * 
 * @return int test result
 */
int page_status_test() {
	TEST_HEADER;
	clear();
	int i, result = PASS;

	/* page directory status test */
	result = ( page_directory[0].KB.present == 1 ) & ( page_directory[1].MB.present == 1 );
	for(i = 2; i < ENTRY_NUM; i++) 
		result &= page_directory[i].MB.present == 0;
	if(result == PASS) printf("Page directory entries status test passed.\n");

	/* page table status test */
	for(i = 0; i < ENTRY_NUM; i++) 
		result &= (i == VIDEO_INDEX) ? page_table[i].present : !page_table[i].present;
	if(result == PASS) printf("Page table entries status test passed.\n");
	return result;
}

/**
 * @brief This function will dereference an available
 * address and return PASS.
 * 
 * @return int test result
 */
int page_access_test() {
	TEST_HEADER;
	int result = PASS;
	int* temp_pt;
	int temp_v;

	/* dereference video memory */
	temp_pt = (int*) (VIDEO_INDEX << PTE_OFFSET);
	temp_v = *temp_pt;
	printf("Video memory dereferenced successfully.\n");

	/* dereference 4MB-8MB memory */
	temp_pt = (int*) MB_4;
	temp_v = *temp_pt;
	printf("4MB memory dereferenced successfully.\n");

	return result;
}

/**
 * @brief This function will dereference an unavailable 
 * address and generate a page fault exception.
 * 
 */
void page_fault_test() {
	TEST_HEADER;
	int* temp_pt;
	int temp_v;

	printf("Dereferencing a null pointer:\n");
	temp_pt = NULL;
	temp_v = *temp_pt;

	printf("Dereferencing random unavailable address:\n");
	temp_pt = (int*) ( (VIDEO_INDEX + 1) << PTE_OFFSET);
	temp_v = *temp_pt;
}

/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests() {
	printf("--------------------------- Test begins ---------------------------\n");
	TEST_OUTPUT("idt_test", idt_test());
	// divide_error();
	syscall_check();
	TEST_OUTPUT("page_status_test", page_status_test());
	TEST_OUTPUT("page_access_test", page_status_test());
	printf("[TEST page_fault_test]: There should be two page fault. \n");
	// page_fault_test();
	printf("---------------------------- Test Ends ----------------------------\n");
}
