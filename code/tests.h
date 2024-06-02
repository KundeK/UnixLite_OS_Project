/* 
 * test.h - Defines testing functions for the code
 */

#ifndef TESTS_H
#define TESTS_H

// test launcher
void launch_tests();

// Checkpoint 1 Tests
int idt_test();
int divide_by_zero_test();
int overflow_test();
int bounds_test();
int system_call_test();
int paging_test_valid();
int paging_test_invalid();

// Checkpoint 2 Tests
int terminal_read_test();
int terminal_write_test();
int rtc_read_test();
int rtc_write_test();
int dir_read_test();

#endif /* TESTS_H */
