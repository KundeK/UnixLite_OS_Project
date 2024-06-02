#ifndef _PROCESSES_H
#define _PROCESSES_H

#include "file.h"
#include "paging.h"
#include "rtc.h"
#include "terminal.h"
#include "file_system.h"
#include "lib.h"
#include "i8259.h"
#include "pit.h"

#define NUM_MAX_FILES           8
#define PCB_SIZE                0x2000
#define PROCESS_ONE_ADDR        0x800000
#define PROCESS_TWO_ADDR        0xC00000
#define PROGRAM_VIR_ADDR        0x8000000
#define PROGRAM_IMG_OFFSET      0x48000
#define PROCESS_PD_INDEX        2
#define CMD_LINE_LEN            128
#define NUM_TERMS               3

#define CURSOR_X_START          7
#define CURSOR_Y_START          1

typedef struct process_control_block {
    fd_t file_descriptor_table[NUM_MAX_FILES];
    int32_t parent_pid;
    uint32_t ebp;
    uint32_t esp;
    uint32_t cur_ebp;
    uint32_t cur_esp;
    uint32_t eflag;
    uint8_t cmd_args[CMD_LINE_LEN];
} pcb_t;

typedef struct terminal {
    int cursor_x;
    int cursor_y;
    int base_pid;
    int active_pid;
    int vidmem_idx;
    uint8_t terminal_buf[CMD_LINE_LEN];
    int buf_idx;
} term_t;

/*
 *  Define file operations tables
 */
f_op_t file_opt;
f_op_t dir_opt;
f_op_t rtc_opt;
f_op_t terminal_opt;
f_op_t stdin_opt;
f_op_t stdout_opt;
f_op_t null_opt;

term_t terminals[NUM_TERMS];

int ts;
int ta;

uint32_t process_init();

int32_t set_active_terminal(int active_term);
int get_active_terminal();
int get_shown_terminal();
void set_ta(int new_ta);

int32_t invalid_open_close(uint8_t* file_name);
int32_t invalid_read_write(int32_t fd, uint8_t* buf, int32_t nbytes);

#endif
