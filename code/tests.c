#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "terminal.h"
#include "rtc.h"
// #include "directory.h"
#include "file.h"
// #include "file_system.h"

#define PASS 1
#define FAIL 0
#define INT_MAX 2147483647

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

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
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

// add more tests here
/* Divide by Zero Test
 * 
 * Asserts that dividing by zero throws an exception
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Throw exception
 */
int divide_by_zero_test(){
	TEST_HEADER;
	int a = 10;
	int b = 0;
	a /= b;
	return FAIL;
}

/* Overflow Test
 * 
 * Asserts that integer overflow throws an exception
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Throw exception
 */
int overflow_test(){
	TEST_HEADER;
	asm volatile ("int $0x04");
	return FAIL;

}

/* Bounds Test
 * 
 * Asserts that going out of bounds throws an exception
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Throw exception
 */
int bounds_test(){
	TEST_HEADER;
	asm volatile ("int $0x05");
	return FAIL;
}

/* System Call Test
 * 
 * Asserts that doing a system call provides a message
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Throw exception
 */
int system_call_test(){
	TEST_HEADER;
	asm volatile ("int $0x80");
	return PASS;
}

/* Paging Valid Test
 * 
 * Tests that show dereferencing locations that should be accessible are accessible
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Accessible memory
 */
int paging_test_valid() {
	TEST_HEADER;

	char result;
	char* ptr = (char*)0x400000;
	result = *ptr;

	ptr = (char*)0x7FFFFF;
	result = *ptr;

	ptr = (char*)0xB8000;
	result = *ptr;

	ptr = (char*)0xB8FFF;
	result = *ptr;

	return PASS;
}

/* Paging Invalid Test
 * 
 * Tests that show that dereferencing locations that shouldn’t be accessible aren’t accessible
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Throw page fault
 */
int paging_test_invalid() {
	TEST_HEADER;

	char result;
	char* ptr = (char*)0x300000;
	result = *ptr;

	ptr = (char*)0x8FFFFF;
	result = *ptr;

	ptr = (char*)0xB7000;
	result = *ptr;

	ptr = (char*)0xB9FFF;
	result = *ptr;

	return FAIL;
}


/* Checkpoint 2 tests */

/* Terminal Read Test
 * 
 * Asserts that reading the keyboard buffer provides a message
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Copying the keyboard buffer
 */
int terminal_read_test(){
	TEST_HEADER;
	int i = 1;
	while(i){
		rtc_read(0, NULL, 0);
		i = 0;
	}
	char arr[128];
	terminal_read(0, (uint8_t*)arr, 128);
	putc('\n');
	puts(arr);
	putc('\n');
	return PASS;
}

/* Terminal Write Test
 * 
 * Asserts that we can write from a buffer to the terminal
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Copying from buffer to terminal
 */
int terminal_write_test() {
	TEST_HEADER;
	char arr[] = {'h', 'e', 'l', 'l', '\n'};
	terminal_write(0, (uint8_t*)arr, 5);
	return PASS;
}

/* RTC Read Test
 * 
 * Asserts that the RTC blocks interrupts
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Blocking interrupts and then shifting screen
 */
int rtc_read_test() {
	TEST_HEADER;
	while(1){
		rtc_read(0, NULL, 0);
	}
	return PASS;
}

/* RTC Write Test
 * 
 * Asserts that we can run the RTC at different frequencies
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Modifying the RTC rate
 */
int rtc_write_test() {
	TEST_HEADER;
	clear_screen();
	int test_frequencies[] = {3,4,5,6,7,8,9,10,11,12,13,14,15};
	int i;
	
	for (i = 12; i >= 0; i--) {
		if (i == 12) {
			rtc_open(NULL);
		}
		else {
			rtc_write(0, NULL, test_frequencies[i]);
		}
		int count = 0;
	    // generate interrupts and print a character
		while(1){
			rtc_read(0, NULL, 0);
			putc('1');
			count++;
			if(count == 40) {
				break;
			}
		}
		clear_screen();
	}
	return PASS;
}

/* Directory Read Test
 * 
 * Asserts that reading the keyboard buffer provides a message
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Copying the keyboard buffer
 */
int dir_read_test() {
	clear_screen();

	TEST_HEADER;
	uint8_t buf[33];

	// confirm each return value is correct
	if (dir_read(0, buf, 10000) != 1) {return FAIL;}
	if (dir_read(0, buf, 10000) != 7) {return FAIL;}
	if (dir_read(0, buf, 10000) != 5) {return FAIL;}
	if (dir_read(0, buf, 10000) != 4) {return FAIL;}
	if (dir_read(0, buf, 10000) != 6) {return FAIL;}
	if (dir_read(0, buf, 10000) != 3) {return FAIL;}
	if (dir_read(0, buf, 10000) != 4) {return FAIL;}
	if (dir_read(0, buf, 10000) != 7) {return FAIL;}
	if (dir_read(0, buf, 10000) != 8) {return FAIL;}
	if (dir_read(0, buf, 10000) != 3) {return FAIL;}
	if (dir_read(0, buf, 10000) != 10) {return FAIL;}
	if (dir_read(0, buf, 10000) != 32) {return FAIL;}
	if (dir_read(0, buf, 10000) != 2) {return FAIL;}
	if (dir_read(0, buf, 10000) != 9) {return FAIL;}
	if (dir_read(0, buf, 10000) != 11) {return FAIL;}
	if (dir_read(0, buf, 10000) != 10) {return FAIL;}
	if (dir_read(0, buf, 10000) != 5) {return FAIL;}
	
	// subsequent reads should return nothing
	if (dir_read(0, buf, 10000) != 0) {return FAIL;}
	if (dir_read(0, buf, 10000) != 0) {return FAIL;}
	if (dir_read(0, buf, 10000) != 0) {return FAIL;}

	return PASS;
}

int dir_open_close_test() {
	TEST_HEADER;
	if (dir_open((uint8_t*)".") != 0) {return FAIL;}
	if (dir_close((uint8_t*)".") != 0) {return FAIL;}
	if (dir_write(0, NULL, 0) != -1) {return FAIL;}

	return PASS;
}

int file_open_close_test() {
	TEST_HEADER;
	if (file_open((uint8_t*)"fish") != 0) {return FAIL;}
	if (file_close((uint8_t*)"fish") != 0) {return FAIL;}
	if (file_write(0, NULL, 0) != -1) {return FAIL;}

	return PASS;
}

int file_read_test() {
	TEST_HEADER;
	static uint8_t buf[100000];

	clear_screen();

	file_open((uint8_t*)"verylargetextwithverylongname.tx");
	file_read(0, buf, 100000);
	puts((int8_t*)buf);

	return PASS;
}

int file_read_exe_test() {
	TEST_HEADER;
	static uint8_t buf[100000];
	int i, j;

	clear_screen();

	file_open((uint8_t*)"cat");
	file_read(0, buf, 100000);
	for (i = 0; i < 15; i++) {
		putc(buf[i]);
	}
	

	while (buf[i] != 'Z') {
		i++;
	}

	for (j = 40; j >= 0; j--) {
		putc(buf[i-j]);
	}


	return PASS;
}

/* Checkpoint 3 tests */
int syscall_jump_test() {
	TEST_HEADER;
	asm volatile (
		"movl $1, %eax;"
		"int $0x80;");
	return PASS;
}

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// Example
	// TEST_OUTPUT("idt_test", idt_test());

	// dir_read_test();
	
	// launch your tests here
	// TEST_OUTPUT("Divide by Zero test", divide_by_zero_test());
	// TEST_OUTPUT("Overflow test", overflow_test());
	// TEST_OUTPUT("Bounds test", bounds_test());
	// TEST_OUTPUT("System Call", system_call_test());
	// TEST_OUTPUT("Paging valid test", paging_test_valid());
	// TEST_OUTPUT("Paging invalid test", paging_test_invalid());

	/*
	 * Checkpoint 2 Tests
	 */
	// TEST_OUTPUT("Terminal Write", terminal_write_test());
	// TEST_OUTPUT("RTC Read", rtc_read_test());
	// TEST_OUTPUT("Terminal Read", terminal_read_test());
	// TEST_OUTPUT("RTC Write", rtc_write_test());

	// TEST_OUTPUT("dir_read_test", dir_read_test());
	// TEST_OUTPUT("File open/read Test", file_read_test());
	// TEST_OUTPUT("File open/read exe Test", file_read_exe_test());
	// TEST_OUTPUT("File Open/Close/Write Test", file_open_close_test());
	// TEST_OUTPUT("Dir Open/Close/Write Test", dir_open_close_test());
	// file_read_test();

	// TEST_OUTPUT("syscall jump test", syscall_jump_test());
}
