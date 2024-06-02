/* 
 * keyboard.h - Defines initialization for the keyboard device
 */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "types.h"
#include "i8259.h"
#include "idt.h"
#include "system_call.h"

// Define port values
#define KB_DATA_PORT    0x60
#define KB_CMD_PORT     0x64
#define KB_IRQ          0x01

// Define commands
#define SET_LEDS        0xED
#define ECHO            0xEE
#define SCAN_CODE       0xF0
#define ID_KB           0xF2
#define ENABLE_SCAN     0xF4
#define DISABLE_SCAN    0xF5
#define DEF_PARAM       0xF6
#define RESEND_BYTE     0xFE
#define RESET_AND_TEST  0xFF
#define SCAN_CODE_1     0x01

// Define response bytes
#define ACK             0xFA
#define RESENT          0xFE
#define KEY_BUF_SIZE    128

// function key values
#define F1      1
#define F2      2
#define F3      3

#define TERM1   0
#define TERM2   1
#define TERM3   2

#define TAB_SPACE 4

/* Externally-visible functions */

/* Initialize keyboard device */
void keyboard_init(void);
void keyboard_handler(void);
int write_char(uint8_t* buf, unsigned int* idx, uint8_t c);
int delete_char(uint8_t* buf, unsigned int* idx);
int ctrl_helper(uint8_t ascii, unsigned int* idx);
int alt_helper(uint8_t ascii, unsigned int* idx);
int terminal_switch(int function);
int clear_keyboard_buf(uint8_t* buf, unsigned int* idx);
int clear_keyboard_buf_caller();
/* Converts the keyboard scan code to a lowercase letter or number */
uint8_t get_key(uint32_t keypress);

/* gets pointer to the keyboard buffer */
uint8_t* get_keyboard_buf();

#endif
