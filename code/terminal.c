#include "terminal.h"
#include "lib.h"
#include "idt.h"
#include "keyboard.h"
#include "processes.h"

// static uint8_t terminal_buf[TERMINAL_BUF_SIZE]; // terminal buffer

/* int terminal_open(void)
 * Inputs: void
 * Return Value: int
 * Function: Initialize the terminal */
int32_t terminal_open(uint8_t* file_name){
    return 0;
}

/* int terminal_close(void)
 * Inputs: void
 * Return Value: int
 * Function: Clears any terminal specific variables */
int32_t terminal_close(uint8_t* file_name){
    return 0;
}

/* int terminal_read(void)
 * Inputs: void
 * Return Value: int
 * Function: Read from the keyboard buffer into buf, return number of bytes read */
int32_t terminal_read(int32_t fd, uint8_t* buf, int32_t nbytes){
    if(buf == NULL || nbytes < 0) {
        return -1;
    }

    int done_typing = 0;
    int i = 0;
    int idx = 0;
    uint8_t* temp;

    // read until newline character
    while (!done_typing) {
        temp = get_keyboard_buf();
        cli();
        for (i = 0; i < KEY_BUF_SIZE && temp[i] != '\0'; i++) {
            if (temp[i] == '\n') {
                done_typing = 1;
                break;
            }
        }
        if(ts != ta) {
            done_typing = 0;
        }
        sti();
    }
    
    cli();

    // copy keyboard buffer to buf
    while( (temp[idx] != '\n') && (idx < nbytes)){
        buf[idx] = temp[idx];
        idx++;
    }
    
    // write newline character
    buf[idx] = temp[idx];
    idx++;

    clear_keyboard_buf_caller();
    sti();
    
    return idx;
}

/* int terminal_write(void)
 * Inputs: void
 * Return Value: int
 * Function: Writes to the screen from buf, return number of bytes read or -1 */
int32_t terminal_write(int32_t fd, uint8_t* buf, int32_t nbytes){
    if(buf == NULL || nbytes < 0) {
        return -1;
    }
    
    int idx = 0;
    while(idx < nbytes){
        putc(buf[idx]);
        idx++;
    }
    return idx;
}
