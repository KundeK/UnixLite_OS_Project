/* 
 * rtc.h - Defines initialization for the rtc device
 */

#ifndef _RTC_H
#define _RTC_H

#include "lib.h"
#include "system_call.h"

// Define port values
#define RTC_IRQ     0x8

#define REG_A       0x8A
#define REG_B       0x8B
#define REG_C       0x0C
#define NMI         0x70
#define RW          0x71
#define BIT_6_MASK  0x40
#define TOP_4_MASK  0xF0

#define MAX_RATE    15
#define MIN_RATE    3

#define MIN_VIR_RATE 6
#define MAX_VIR_RATE 15

#define CURSOR_RATE  256

#define MAX_FREQ    32768
#define MAX_VIR_FREQ    1024
#define MIN_VIR_FREQ    2

/* Initialize RTC device */
void rtc_init();
void rtc_handler(void);

/* RTC Driver Functions */
int32_t rtc_open(uint8_t* file_name);
int32_t rtc_close(uint8_t* file_name);
int32_t rtc_read(int32_t fd, uint8_t* buf, int32_t nbytes);
int32_t rtc_write(int32_t fd, uint8_t* buf, int32_t rate);

void set_rtc_wait();  // setter for the rtc_wait variable

#endif
