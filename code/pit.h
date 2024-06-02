#ifndef _PIT_H
#define _PIT_H

#include "lib.h"
#include "i8259.h"
#include "idt.h"
#include "system_call.h"
#include "processes.h"

#define CHANNEL0_PORT       0x40
#define CHANNEL1_PORT       0x41
#define CHANNEL2_PORT       0x42
#define MODE_REG_PORT       0x43

#define IRQ0_PORT           0x20

#define PIT_INT_FREQ        25000
#define INT_TIME_MS         30
#define PIT_REL_VAL         INT_TIME_MS * 3579545 / 3000

#define LOW_BYTE_MASK       0xFF
#define HIGH_BYTE_SHIFT     8

#define MODE_4_SELECT       0x34    // 0011 0100 
#define MODE_3_SELECT       0x33    // 0011 0011

#define TOTAL_TERM  3

void pit_init();
void pit_handler();

int term_init_done;

#endif
