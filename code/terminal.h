#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "lib.h"

// Useful constants for the terminal drivers
#define NUM_COLS    80
#define NUM_ROWS    25
#define TERMINAL_BUF_SIZE   NUM_ROWS*NUM_COLS

/* Driver Functions */
int32_t terminal_open(uint8_t* file_name);
int32_t terminal_close(uint8_t* file_name);
int32_t terminal_read(int32_t fd, uint8_t* buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, uint8_t* buf, int32_t nbytes);

#endif
