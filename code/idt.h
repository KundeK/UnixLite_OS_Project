/* 
 * idt.h - Defines initialization for the Interrupt Descriptor Table
 */

#ifndef _IDT_H
#define _IDT_H

#include "x86_desc.h"
#include "types.h"
#include "exception.h"

// IDT indexing numbers
#define NUM_EXCEPTIONS      32
#define SYSCALL_VEC_NUM     0x80
#define INTEL_RESERVED      0x0F
#define NUM_VEC             256
#define KEYBOARD_VEC_NUM    0x21
#define RTC_VEC_NUM         0x28
#define NO_ERR_CODE         -1
#define DATA_PORT           0x60
#define IRQ1                0x1
#define IRQ8                0x8
#define IRQ0                0x0
#define PIT_VEC_NUM         0x20

#define EXCEPTION_HALT      256

/* Function to initialize the IDT and set the necessary handlers */
void initialize_idt();

/* Exception handler to write the exception message and associated error code when there's an exception */
void handle_exception(uint32_t id, uint32_t err_code);

#endif
