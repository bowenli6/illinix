#include <boot/x86_desc.h>
#include <lib.h>
#include <io.h>
#include <drivers/terminal.h>
#include <drivers/rtc.h>
#include <tests/tests.h>
#include <vfs/vfs.h>
#include <boot/syscall.h>
#include <boot/page.h>
#include <kmalloc.h>

	
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
 * @brief Observe if the divide_error exception handler 
 * prints the error message.
 * 
 */
void divide_error() {
	TEST_HEADER;
	int one = 1;
	int zero = 0;
	int check = one / zero;
	printf("I don't like warning, so I don't care what %d is\n", check);
}

/**
 * @brief Observe if the general_protection exception handler
 * prints the error message.
 * 
 */
void inline general_protection() {
	TEST_HEADER;
	asm volatile("iret");
}



/**
 * @brief Observe if the syscall handler
 * print 3 massages.
 * 
 */
void inline syscall() {
	TEST_HEADER;
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
// int page_status_test() {
// 	TEST_HEADER;
// 	int i, result = PASS;

// 	/* page directory status test */
// 	result = ( page_directory[0].KB.present == 1 ) & ( page_directory[1].MB.present == 1 );
// 	for(i = 2; i < ENTRY_NUM; i++) 
// 		result &= page_directory[i].MB.present == 0;
// 	if(result == PASS) printf("Page directory entries status test passed.\n");

// 	/* page table status test */
// 	for(i = 0; i < ENTRY_NUM; i++) 
// 		result &= (i == VIDEO_INDEX) ? page_table[i].present : !page_table[i].present;
// 	if(result == PASS) printf("Page table entries status test passed.\n");
// 	return result;
// }

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
	temp_pt = (int*) (MB_4 * 2 + 8);
	temp_v = *temp_pt;
	printf("4MB memory dereferenced successfully.\n");

	/* dereference 8MB - 8 memory */
	temp_pt = (int*) ( 2 * MB_4 - 8 );
	temp_v = *temp_pt;
	printf("8MB - 8 memory dereferenced successfully.\n");

	return result;
}

/**
 * @brief This function will dereference a null 
 * pointer and generate a page fault exception.
 * 
 */
void page_fault_null() {
	TEST_HEADER;
	int* temp_pt;
	int temp_v;

	printf("Dereferencing a null pointer:\n");
	temp_pt = NULL;
	temp_v = *temp_pt;
}

/**
 * @brief This function will dereference an unavailable 
 * address and generate a page fault exception.
 * 
 */
void page_fault_video_table() {
	TEST_HEADER;
	int* temp_pt;
	int temp_v;

	printf("Dereferencing an unavailable address in page table:\n");
	temp_pt = (int*) ( (VIDEO_INDEX + 1) << PTE_OFFSET);
	temp_v = *temp_pt;
}

/**
 * @brief This function will dereference an unavailable 
 * address and generate a page fault exception.
 * 
 */
void page_fault_out_range() {
	TEST_HEADER;
	int* temp_pt;
	int temp_v;

	printf("Dereferencing unavailable address 8MB:\n");
	temp_pt = (int*) ( MB_4 * 2 );
	temp_v = *temp_pt;
}


/**
 * @brief The tests for checkpoint 1.
 * 
 */
void test_checkpoint1() {
	clear();
	TEST_OUTPUT("idt_test", idt_test());

	/* Exceptions */
	// divide_error();
	// general_protection();

	/* System calls */
	syscall();

	/* page test 1 */
	// TEST_OUTPUT("page_status_test", page_status_test());
	// TEST_OUTPUT("page_access_test", page_access_test());

	/* page test 2 */
	printf("[TEST page_fault_null]: There should be a page fault. \n");
	page_fault_null();

	/* page test 3 */
	// printf("[TEST page_fault_video_table]: There should be a page fault. \n");
	// page_fault_video_table();

	/* page test 4 */
	// printf("[TEST page_fault_out_range]: There should be a page fault. \n");
	// page_fault_out_range();

}


/* Checkpoint 2 tests */

void pre_test() {
	char buf[10];
	puts("\nAre you ready for the next test? (Please type y) \n");
	int in = terminal_open("stdin");
	while (strncmp(buf, "y", 1)) {
		memset((void*)buf, 0, 10);
		terminal_read(in, (void*)buf, 10);
	}
	terminal_close(in);
	clear();
}

/**
 * @brief Test for terminal_read()
 * 
 */
void test_terminal_read1() {
	TEST_HEADER;
	char buf[10];
	int nread;
	int fd = terminal_open("stdin");
	puts("Welcome to our checkpoint 2 test! Hope you like it :)\n");
	puts("For this test, the terminal will read up to 10 characters (including the new-line character!)\n");
	puts("The purpose of this test is first to let you check the basic functionality of the terminal. \n");
	puts("Please remember if you type more than 10 characters, the termimal will only give you the top 10, and save the remaining for the next read.\n");
	puts("If you want to break from the test loop, simply type q.\n");
	puts("Enjoy!\n");
	while (1) {
		memset((void*)buf, 0, 10);
		puts("Please type your data:\n");
		nread = terminal_read(fd, (void*)buf, 10);
		if (!strncmp(buf, "q", 1))
			break;
		printf("The number of bytes read: %d\n", nread);
		printf("The data read is: %s\n", buf);
	}
	terminal_close(fd);
	pre_test();
}

void test_terminal_read2() {
	TEST_HEADER;
	char buf[10];
	int fd = terminal_open("stdin");
	puts("You should change the buffer size to 5 for this test.\n");
	puts("This test is for testing the max buffer size.\n");
	puts("The size of our input line buffer is limited.");
	puts("So if you type more than 5, the buffer can only save the lastest 5 characters.\n");
	puts("And if also, if you want more than 5 characters read, sorry, it can only give you 5.\n");
	puts("If you want to break from the test loop, simply type quit.\n");
	puts("Enjoy!\n");
    memset((void*)buf, 0, 10);
	puts("Please type your data:\n");
	printf("The number of bytes read: %d\n", terminal_read(fd, (void*)buf, 10));
	printf("The data read is: %s\n", buf);
	terminal_close(fd);
	pre_test();
}


void test_terminal_write1() {
	TEST_HEADER;
	char buf[10];
	int nread, nwrite;
	int in = terminal_open("stdin");
	int out = terminal_open("stdout");
	puts("This test is build for testing terminal writes.\n");
	puts("The test wants you to type someting from the stdin, and it will use the terminal_write function to acho it back to the stdout.\n");
	puts("The size it wants is 5 characters (including new-line character).\n");
	puts("If you want to break from the test loop, simply type q.\n");
	puts("Enjoy!\n");
	while (1) {
		memset((void*)buf, 0, 10);
		puts("Please type your data:\n");
		nread = terminal_read(in, (void*)buf, 5);
		if (!strncmp(buf, "q", 1))
			break;
		nwrite = terminal_write(out, (void*)buf, nread);
		printf("The number of bytes read: %d\n", nread);
		printf("The number of bytes write: %d\n", nwrite);
	}
	terminal_close(in);
	terminal_close(out);
	pre_test();
}


void test_terminal_write2() {
	TEST_HEADER;
	char buf[10];
	int out = terminal_open("stdout");	
	puts("For this test, you will test what happens if you tell it to write a number of bytes that doesn't match the buffer size.\n");
	strcpy(buf, "hello");
	puts("The first case: I will write hello to the screen with a given nbytes 10(which should be mismatched.)\n");
	printf("The number of bytes write: %d\n", terminal_write(out, (void*)buf, 10));
	puts("-----------------------------------------------------------\n");
	puts("The next case: I will write hello to the screen with a given nbytes 3(which should be mismtached.)\n");
	printf("The number of bytes write: %d\n", terminal_write(out, (void*)buf, 3));
	terminal_close(out);
	pre_test();
}


static void print_newline(int number) {
	while(number--) putc(' ');
}

void test_directory_ls() {
	TEST_HEADER;
	int32_t fd, cnt;
    char buf[33];
	int i;
	int len;
	dentry_t d;
	int in = terminal_open("stdin");
	int out = terminal_open("stdout");
	const char *filename[17] = {
		".", "sigtest", "shell", "grep", "syserr", "rtc", 
		"fish", "counter", "pingpong", "cat","frame0.txt", 
		"verylargetextwithverylongname.txt", "ls", 
		"testprint", "created.txt", "frame1.txt", "hello"
	};
	puts("The test is for testing directory open, read, and close.\n");
	puts("It works like ls, which list all the files currently located in the directory.\n");
	puts("Notice that the largest file name is not printed normally because it exceeds the file name size limit.\n");
    puts("Are you ready to see the result? If so, type y. \n");
	while (strncmp(buf, "y", 1)) {
		memset((void*)buf, 0, 10);
		terminal_read(in, (void*)buf, 10);
	}
	clear();
	if (-1 == (fd = directory_open ((int8_t*)"."))) {
        puts("directory open failed\n");
    }
	puts("File names");
	print_newline(30);
	puts("File type");
	print_newline(15);
	puts("File size\n");

	i = 0;
    while (0 != (cnt = directory_read (fd, buf, 32))) {
        if (-1 == cnt) {
	        puts ("directory entry read failed\n");
			break;
	    }
		len = (int32_t)strlen(filename[i]);
		terminal_write(out, buf, len);
		read_dentry_by_name(filename[i], &d);
		print_newline(abs(len - 40));
		printf("%d", d.type);
		print_newline(8 + 15);
		printf("%d\n", (int)get_size(i++));
		memset((void*)buf, 0, 33);
	}
	directory_close(fd);
	terminal_close(in);
	terminal_close(out);
	pre_test();
}

// const char *filename[16] = {
	// 	"sigtest", "shell", "grep", "syserr", "rtc", 
	// 	"fish", "counter", "pingpong", "cat","frame0.txt", 
	// 	"verylargetextwithverylongname.txt", "ls", 
	// 	"testprint", "created.txt", "frame1.txt", "hello"
	// };
void test_file_read() {
	TEST_HEADER;
	int exit;
	char buf[10];
	char fname[33];
	char filebuf[10000];
	int fd, nread, size;
	dentry_t d;
	int in = terminal_open("stdin");
	int out = terminal_open("stdout");
	puts("This test will read every regular file from the file system.\n");
	puts("For every file, you need to type read to get the next file printed to the screen.\n");
	puts("Enjoy!\n");

	while (1) {
		puts("Type the file name you want to read, (exit to quit): \n");
		memset((void*)fname, 0, 33);
		nread = terminal_read(in, (void*)fname, 32);
		if (!strncmp(fname, "exit", 4))
			break;
		fname[nread-1] = '\0';
		if ((fd = file_open(fname)) < 0) {
			printf("Failed to open [%s]\n", fname);
			continue;
		}
		read_dentry_by_name(fname, &d);
		size = fs->inodes[d.inode].size;
		printf("The file you have just open is: %s\n", fname);
		printf("The type of this file is(0: RTC, 1: Directory, 2: Regular file): %d\n", d.type);
		printf("The correct size of this file is : %d\n", size);
		exit = 1;
		while (size && exit && (nread = file_read(fd, filebuf, 1000))) {
			if (nread == -1) {
				printf("Failed to read from [%s]\n", fname);
				exit = 0;
				file_close(fd);
				break;
			}
			clear();
			printf("The number of bytes we read is : %d\n", nread);
			size -= nread;
			printf("The Remaining number of bytes in the file is : %d\n", size);
			puts("Please type s to see the file: \n");
			while (strncmp(buf, "s", 1)) {
				memset((void*)buf, 0, 10);
				terminal_read(in, (void*)buf, 10);
			}
			memset((void*)buf, 0, 10);
			terminal_write(out, filebuf, nread);
			puts("Do you want to see the remaining content of this file? (y or n) \n");
			while (1) {
				memset((void*)buf, 0, 10);
				terminal_read(in, (void*)buf, 10);
				if (!strncmp(buf, "n", 1)) {
					exit = 0;
					break;
				} else {
					break;
				}
			}
			memset((void*)buf, 0, 10);
			memset((void*)filebuf, 0, 1000);
		}
		file_close(fd);
	}
	terminal_close(in);
	terminal_close(out);
	pre_test();
}

/**
 * @brief Test for RTC read and write
 * 
 */
int test_RTC_ReadWrite(){
	TEST_HEADER;
	int i;
	int fd;
	int32_t freq;
	// initalize the RTC
	fd = rtc_open ("RTC");	
	// every time frequency = frequency * 2	
	for(freq = RTC_MIN_freq; freq <= RTC_MAX_freq; freq *= 2) {
		for(i = 0; i < freq; i++) {
			// wait on interrupt generation
			rtc_read(fd, NULL, 0);
			printf("%d", 1);
		}
		// set new frequency
		rtc_write(fd, (void*)(&freq), sizeof(int32_t));
		clear();
	}
	rtc_close(NULL);
	return PASS;
}



void test_keyboard_adv() {
	puts("Test keyboard:\n");
	while(1);
}


/**
 * @brief The tests for checkpoint 2.
 * 
 */
void test_checkpoint2() {
	clear();
	// test_terminal_read1();
	// test_terminal_read2();
	// test_terminal_write1();
	// test_terminal_write2();
	// test_directory_ls();
	// test_file_read();
	// test_RTC_ReadWrite();
	test_keyboard_adv();
}



/* Checkpoint 3 tests */

// int test_shell() {
// 	int32_t cnt, rval;
// 	const int32_t BUFSIZE = 1024;
//     int8_t buf[BUFSIZE];
// 	int errno;

//     if ((errno = fputs(stdout, "Starting illinix Shell\n")) < 0) {
// 		fputs(stdout, "Error occurs\n");
// 	}

//     while (1) {
//         fputs (stdout, "illinix> ");
// 		if (-1 == (cnt = sys_read (0, buf, BUFSIZE-1))) {
// 			fputs (1, "read from keyboard failed\n");
// 			return 3;
// 		}
// 		if (cnt > 0 && '\n' == buf[cnt - 1])
// 			cnt--;
// 		buf[cnt] = '\0';
// 		if (0 == strncmp (buf, "exit", 4))
// 			return 0;
// 		if ('\0' == buf[0])
// 			continue;
		// rval = sys_execute (buf);
		// if (-1 == rval)
		// 	fputs (1, "no such command\n");
		// else if (256 == rval)
		// 	fputs (1, "program terminated by exception\n");
		// else if (0 != rval)
		// 	fputs (1, "program terminated abnormally\n");
// 	}
// }

int testprint () {
	fputs(1, "Hello World!\n");
    // fputs (1, "Hello, if this ran, the program was correct. Yay!\n");
    return 0;
}




int test_checkpoint3() {
	// testprint();
	// test_file_read();
	char fname[32];
	int nread;
	terminal_open(0);
	nread = terminal_read(0, (void *)fname, 32);
	printf("%d\n", nread);
	printf("The filename is: %s\n", fname);
	return 0;
}

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

void test_kmalloc() {
	int temp, t2, t3;
    printf("kmalloc 4MB: %x\n", t2 = (uint32_t)kmalloc(PAGE_SIZE_4MB));
    printf("kmalloc 123: %x\n", temp = (uint32_t)kmalloc(123));
    //kfree((void*)temp);
	//printf("kfree 123\n");
    kfree((void*)t2);
	printf("kfree 4MB\n");
    printf("kmalloc 8KB: %x\n", (uint32_t)kmalloc(PAGE_SIZE*2));
    printf("kmalloc 1MB: %x\n", t3 = (uint32_t)kmalloc(PAGE_SIZE_4MB/4));
    printf("kmalloc 2MB: %x\n", (uint32_t)kmalloc(PAGE_SIZE_4MB/2));
	//kfree((void*)t3);
	printf("kree 1MB\n");
    printf("kmalloc 513: %x\n", (uint32_t)kmalloc(513));
    printf("kmalloc 1024: %x\n", (uint32_t)kmalloc(1024));
    printf("kmalloc 2048: %x\n", (uint32_t)kmalloc(1024));
    printf("kmalloc 4KB: %x\n", (uint32_t)kmalloc(PAGE_SIZE));
	printf("kmalloc 8KB: %x\n", (uint32_t)kmalloc(2 * PAGE_SIZE));
	printf("kmalloc 4KB: %x\n", (uint32_t)kmalloc(PAGE_SIZE));

	//int u1, u2, u3;
	// vmem_t vm1, vm2;
	// int* addr = (int*)USER_MEM;
	// vm1.size = PAGE_SIZE * 4;
	// printf("umalloc 4MB, 0 means succeed: %x\n", vmalloc(&vm1, 0, PAGE_SIZE * 4, PTE_US | PTE_RW));
	// *addr = 0x1234;
	// vmcopy(&vm2, &vm1);
	// printf("umalloc 8KB, 0 means succeed: %x\n", vmalloc(HEAP_START, 0, PAGE_SIZE * 2, PTE_US | PTE_RW));
	// free_user_page(u1, 10);
	// printf("umalloc 2MB: %x\n", get_user_page(9));
	// printf("umalloc 1MB: %x\n", get_user_page(8));
}

/* Test suite entry point */
void launch_tests() {
	printf("--------------------------------- Test begins ---------------------------------\n");
	// test_checkpoint1();
	// test_checkpoint2();
	//page_access_test();
	//test_checkpoint3();
	test_kmalloc();
	printf("---------------------------------- Test Ends ----------------------------------\n");
}
