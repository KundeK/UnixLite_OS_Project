#include "keyboard.h"
#include "lib.h"
#include "idt.h"
#include "terminal.h"
#include "processes.h"
#include "paging.h"
#include "system_call.h"
#include "pit.h"

// Keyboard global variables
int tab = 0;
int caps = 0;
int l_shift = 0;
int r_shift = 0;
int ctrl = 0;
int alt = 0;
int backspace = 0;
int enter_flag = 0;
int function = 0;

static uint8_t keyboard_buf[KEY_BUF_SIZE];
unsigned int key_idx = 0;


/* void keyboard_init(void)
 * Inputs: void
 * Return Value: void
 * Function: Initialize the keyboard by enabling the IRQ1 line to the PIC */
void keyboard_init(void) {
    enable_irq(KB_IRQ);         // Enable IRQ1             
}

/* void keyboard_handler(void)
 * Inputs: void
 * Return Value: void
 * Function: Handle the keyboard interrupts */
void keyboard_handler(void) {
    cli();
    uint32_t keypress;
    char ascii;
    keypress = inb(DATA_PORT);
    ascii = get_key(keypress);

    //Unit test each part first
    if(ctrl == 1){
        ctrl_helper(ascii, &key_idx);
    }
    else if(ascii != 0x00){
        int success = 0;

        success = write_char(keyboard_buf, &key_idx, ascii);
        if(success == 0){
            if(ascii == '\n') {
                check_cursor();     // Make sure to undraw the cursor
            }
            put_key(ascii);
        }
    } 
    else if (backspace == 1) {
        if( delete_char(keyboard_buf, &key_idx) == 0 ){
            remc();
            check_cursor();         // Make sure to undraw the cursor
        }
    } 
    else if (tab == 1) {
        int i = 0;
        while( (i < TAB_SPACE) && (write_char(keyboard_buf, &key_idx, ' ') == 0) ){
            put_key(' ');
            i++;
        }
    }
    else if(alt == 1) {
        send_eoi(IRQ1);
        alt_helper(ascii, &key_idx);
    }

    send_eoi(IRQ1);
    sti();
}


/* int write_char(uint8_t* buf, unsigned int* idx, uint8_t c)
 * Inputs:  uint8_t* buf - buffer to write character into
 *          unsigned int* idx - pointer to current index inside the buffer
 *          uint8_t c - character to write into the buffer
 * Return Value: 0 on success, -1 on failure
 * Function: writes a character into buf at index idx (i.e. buf[*idx] = c)
 */
int write_char(uint8_t* buf, unsigned int* idx, uint8_t c){
    if(*idx == KEY_BUF_SIZE - 1 && c != '\n') {
        return -1;
    }
    
    if(*idx >= KEY_BUF_SIZE || idx == NULL){
        return -1;
    }

    buf[*idx] = c;      // set char at current idx to c
    *idx = *idx + 1;    // increment buf index position
    return 0;
}

/* int delete_char(uint8_t* buf, unsigned int* idx, uint8_t c)
 * Inputs:  uint8_t* buf - buffer to write character into
 *          unsigned int* idx - pointer to current index inside the buffer
 * Return Value: 0 on success, -1 on failure
 * Function: deletes most recently added char from buf at index idx (i.e. buf[*idx] = '\0')
 */
int delete_char(uint8_t* buf, unsigned int* idx){
    if(*idx <= 0 || idx == NULL){
        return -1;
    }

    *idx = *idx - 1;          // decrement buf index position
    buf[*idx] = '\0';        // set char at cursor position to NULL char
    return 0;
}

/* int ctrl_helper(uint8_t c)
 * Inputs:  uint8_t c - other character pressed in conjuction with ctrl key
 * Return Value: 0 if shortcut exists, -1 if no shortcut exists
 * Function: performs ctrl based keyboard shortcut
 */
int ctrl_helper(uint8_t c, unsigned int* idx){
    switch(c){
        // CTRL+L clears the screen
        case 'l':
            clear_screen(); 
            clear_keyboard_buf(get_keyboard_buf(), idx); 
            return 0;
        case 'L': 
            clear_screen();
            clear_keyboard_buf(get_keyboard_buf(), idx); 
            return 0;
        case '\0':
            return 0;
        default:
            write_char(keyboard_buf, idx, c);
            put_key(c);
            return 0;

    }

    return -1;
}

/* int alt_helper(uint8_t ascii, unsigned int* idx)
 * Inputs:  uint8_t c - other character pressed in conjuction with ctrl key
 *          unsigned int* idx - pointer to index tracking variable
 * Return Value: 0 terminal successfully switches, -1 if switched terminal is same as active
 * Function: switches shown terminal on screen
 */
int alt_helper(uint8_t ascii, unsigned int* idx){
    if( function < F1 || function > F3) {
        return -1;
    }

    return terminal_switch(function);
}

/* int terminal_switch(int function)
 * Inputs:  int function - describes which function key is active
 * Return Value: 0 terminal successfully switches, -1 if switched terminal is same as active
 * Function: switches shown terminal on screen
 */
int terminal_switch(int function){
    cli();
    int vid_mem = VID_MEM_INDEX << OFFSET_12;  // video memory at 0xB8000
    int prev_vid_addr;
    int vid_addr;
    int prev_ts;

    // find previous terminal's vid memory
    prev_ts = ts;
    if(prev_ts == TERM1){
        prev_vid_addr = TERMINAL_ONE_VIDMEM_IDX << OFFSET_12;  // 0xB9000
    } else if(prev_ts == TERM2){
        prev_vid_addr = TERMINAL_TWO_VIDMEM_IDX << OFFSET_12;  // 0xBA000
    } else if(prev_ts == TERM3){
        prev_vid_addr = TERMINAL_THREE_VIDMEM_IDX << OFFSET_12; // 0xBB000
    }
    
    // find new terminal's vid memory and update ts
    if(function == F1) {
        ts = TERM1;
        vid_addr = TERMINAL_ONE_VIDMEM_IDX << OFFSET_12;
    } else if (function == F2) {
        ts = TERM2;
        vid_addr = TERMINAL_TWO_VIDMEM_IDX << OFFSET_12;
    } else if (function == F3) {
        ts = TERM3;
        vid_addr = TERMINAL_THREE_VIDMEM_IDX << OFFSET_12;
    }

    // switching to the same terminal so don't do anything
    if(prev_ts == ts){
        return -1;
    }
    
    // update previous terminals struct
    int i;
    for(i = 0; i < KEY_BUF_SIZE; i++) {
        terminals[prev_ts].terminal_buf[i] = keyboard_buf[i];
    }
    terminals[prev_ts].buf_idx = key_idx;

    // update physical variables from new terminal's struct
    for(i = 0; i < KEY_BUF_SIZE; i++) {
        keyboard_buf[i] = terminals[ts].terminal_buf[i];
    }
    key_idx = terminals[ts].buf_idx;

    memcpy((void*)prev_vid_addr, (void*)vid_mem, CONST_4KB);
    memcpy((void*)vid_mem, (void*)vid_addr, CONST_4KB);

    if(term_init_done) {
        if (ts == ta) {
            set_vidmem_page(0);
            pt_user[VID_MEM_INDEX].addr_31_12 = (int)VID_MEM_INDEX;
        }
        else {
            set_vidmem_page(ta+1);
            pt_user[VID_MEM_INDEX].addr_31_12 = (int)(VID_MEM_INDEX + ta + 1);
        }

        // flush TLB
        asm volatile("movl %cr3, %eax;"
                    "movl %eax, %cr3");
    }

    sti();
    return 0;
}

/* int clear_keyboard_buf(uint8_t* buf, unsigned int* idx)
 * Inputs:  uint8_t* buf - buffer to write character into
 *          unsigned int* idx - pointer to current index inside the buffer
 * Return Value: 0 on success, -1 on failure
 * Function: clears entire buf and resets idx value to 0
 */
int clear_keyboard_buf(uint8_t* buf, unsigned int* idx){
    int i;
    for(i = 0; i < KEY_BUF_SIZE; i++){
        buf[i] = '\0';     // initialize all entries in the buf to '\0'
    }
    *idx = 0;
    return 0;
}

/* int clear_keyboard_buf_caller()
 * Inputs: void
 * Return Value: 0 on success, -1 on failure
 * Function: external caller that clears entire buf and resets idx value to 0
 */
int clear_keyboard_buf_caller() {
    return clear_keyboard_buf(keyboard_buf, &key_idx);
}

/* uint8_t* get_keyboard()
 * Inputs: void
 * Return Value: uint8_t*
 * Function: A getter for returning the keyboard buffer */
uint8_t* get_keyboard_buf(){
    return keyboard_buf;
}

/* char get_key(uint32_t keypress)
 * Inputs: uint32_t keypress
 * Return Value: char
 * Function: Convert keyboard scan code input to an ASCII output */
uint8_t get_key(uint32_t keypress){
    int shift = l_shift || r_shift;
    switch((caps << 1) + shift){
        // Lowercase values
        case 0:
            switch(keypress) {
                // numbers pressed
                case 0x29: return '`';  // ` pressed
                case 0x02: return '1';  // 1 pressed
                case 0x03: return '2';  // 2 pressed
                case 0x04: return '3';  // 3 pressed
                case 0x05: return '4';  // 4 pressed
                case 0x06: return '5';  // 5 pressed
                case 0x07: return '6';  // 6 pressed
                case 0x08: return '7';  // 7 pressed
                case 0x09: return '8';  // 8 pressed
                case 0x0A: return '9';  // 9 pressed
                case 0x0B: return '0';  // 0 pressed
                case 0x0C: return '-';  // - pressed
                case 0x0D: return '=';  // = pressed
                /* backspace should be here - 0x0E */
                case 0x0E: backspace = 1; return 0x00;  /* backspace pressed */
                case 0x8E: backspace = 0; return 0x00;  /* backspace released */

                // first row of letters
                case 0x0F: tab = 1; return 0x00;    /* tab pressed */
                case 0x8F: tab = 0; return 0x00;    /* tab released */
                case 0x10: return 'q';  // Q pressed
                case 0x11: return 'w';
                case 0x12: return 'e';
                case 0x13: return 'r';
                case 0x14: return 't';
                case 0x15: return 'y';
                case 0x16: return 'u';
                case 0x17: return 'i';
                case 0x18: return 'o';
                case 0x19: return 'p';
                case 0x1A: return '[';
                case 0x1B: return ']';
                case 0x2B: return '\\';

                // second row
                case 0x3A: caps ^= 1; return 0x00;      /* toggle caps lock */
                case 0x1E: return 'a';
                case 0x1F: return 's';
                case 0x20: return 'd';
                case 0x21: return 'f';
                case 0x22: return 'g';
                case 0x23: return 'h';
                case 0x24: return 'j';
                case 0x25: return 'k';
                case 0x26: return 'l';
                case 0x27: return ';';
                case 0x28: return '\'';
                /* enter should be here - 0x1C */
                case 0x1C: return '\n';

                // third row
                case 0x2A: l_shift = 1; return 0x00;    /* left shift pressed */
                case 0xAA: l_shift = 0; return 0x00;    /* left shift released */
                case 0x2C: return 'z';
                case 0x2D: return 'x';
                case 0x2E: return 'c';
                case 0x2F: return 'v';
                case 0x30: return 'b';
                case 0x31: return 'n';
                case 0x32: return 'm';
                case 0x33: return ',';
                case 0x34: return '.';
                case 0x35: return '/';
                case 0x36: r_shift = 1; return 0x00;    /* right shift pressed */
                case 0xB6: r_shift = 0; return 0x00;    /* right shift released */

                // fourth row
                case 0x1D: ctrl = 1; return 0x00;     /* left/right control pressed */
                case 0x9D: ctrl = 0; return 0x00;     /* left/right control released */
                case 0x38: alt = 1; return 0x00;      /* left/right alt pressed */
                case 0xB8: alt = 0; return 0x00;      /* left/right alt released */
                case 0x39: return ' ';

                // function keys pressed
                case 0x3B: function = F1; return 0x00;
                case 0x3C: function = F2; return 0x00;
                case 0x3D: function = F3; return 0x00;
                // function key unpressed
                case 0xBB: function = 0; return 0x00;
                case 0xBC: function = 0; return 0x00;
                case 0xBD: function = 0; return 0x00;

                default: return 0x00;
            }
        
        // Shift pressed
        case 1:
            switch(keypress) {
                // numbers pressed
                case 0x29: return '~';  // ` pressed
                case 0x02: return '!';  // 1 pressed
                case 0x03: return '@';  // 2 pressed
                case 0x04: return '#';  // 3 pressed
                case 0x05: return '$';  // 4 pressed
                case 0x06: return '%';  // 5 pressed
                case 0x07: return '^';  // 6 pressed
                case 0x08: return '&';  // 7 pressed
                case 0x09: return '*';  // 8 pressed
                case 0x0A: return '(';  // 9 pressed
                case 0x0B: return ')';  // 0 pressed
                case 0x0C: return '_';  // - pressed
                case 0x0D: return '+';  // = pressed
                /* backspace should be here - 0x0E */
                case 0x0E: backspace = 1; return 0x00;  /* backspace pressed */
                case 0x8E: backspace = 0; return 0x00;  /* backspace released */

                // first row of letters
                case 0x0F: tab = 1; return 0x00;    /* tab pressed */
                case 0x8F: tab = 0; return 0x00;    /* tab released */
                case 0x10: return 'Q';  // Q pressed
                case 0x11: return 'W';
                case 0x12: return 'E';
                case 0x13: return 'R';
                case 0x14: return 'T';
                case 0x15: return 'Y';
                case 0x16: return 'U';
                case 0x17: return 'I';
                case 0x18: return 'O';
                case 0x19: return 'P';
                case 0x1A: return '{';
                case 0x1B: return '}';
                case 0x2B: return '|';

                // second row
                case 0x3A: caps ^= 1; return 0x00;      /* toggle caps lock */
                case 0x1E: return 'A';
                case 0x1F: return 'S';
                case 0x20: return 'D';
                case 0x21: return 'F';
                case 0x22: return 'G';
                case 0x23: return 'H';
                case 0x24: return 'J';
                case 0x25: return 'K';
                case 0x26: return 'L';
                case 0x27: return ':';
                case 0x28: return '\"';
                /* enter should be here - 0x1C */
                case 0x1C: return '\n';

                // third row
                case 0x2A: l_shift = 1; return 0x00;    /* left shift pressed */
                case 0xAA: l_shift = 0; return 0x00;    /* left shift released */
                case 0x2C: return 'Z';
                case 0x2D: return 'X';
                case 0x2E: return 'C';
                case 0x2F: return 'V';
                case 0x30: return 'B';
                case 0x31: return 'N';
                case 0x32: return 'M';
                case 0x33: return '<';
                case 0x34: return '>';
                case 0x35: return '?';
                case 0x36: r_shift = 1; return 0x00;    /* right shift pressed */
                case 0xB6: r_shift = 0; return 0x00;    /* right shift released */

                // fourth row
                case 0x1D: ctrl = 1; return 0x00;     /* left/right control pressed */
                case 0x9D: ctrl = 0; return 0x00;     /* left/right control released */
                case 0x38: alt = 1; return 0x00;      /* left/right alt pressed */
                case 0xB8: alt = 0; return 0x00;      /* left/right alt released */
                case 0x39: return ' ';

                // function keys pressed
                case 0x3B: function = F1; return 0x00;
                case 0x3C: function = F2; return 0x00;
                case 0x3D: function = F3; return 0x00;
                // function key unpressed
                case 0xBB: function = 0; return 0x00;
                case 0xBC: function = 0; return 0x00;
                case 0xBD: function = 0; return 0x00;

                default: return 0x00;
            }

        // Capslock toggled
        case 2:
            switch(keypress) {
                // numbers pressed
                case 0x29: return '`';  // ` pressed
                case 0x02: return '1';  // 1 pressed
                case 0x03: return '2';  // 2 pressed
                case 0x04: return '3';  // 3 pressed
                case 0x05: return '4';  // 4 pressed
                case 0x06: return '5';  // 5 pressed
                case 0x07: return '6';  // 6 pressed
                case 0x08: return '7';  // 7 pressed
                case 0x09: return '8';  // 8 pressed
                case 0x0A: return '9';  // 9 pressed
                case 0x0B: return '0';  // 0 pressed
                case 0x0C: return '-';  // - pressed
                case 0x0D: return '=';  // = pressed
                /* backspace should be here - 0x0E */
                case 0x0E: backspace = 1; return 0x00;  /* backspace pressed */
                case 0x8E: backspace = 0; return 0x00;  /* backspace released */

                // first row of letters
                case 0x0F: tab = 1; return 0x00;    /* tab pressed */
                case 0x8F: tab = 0; return 0x00;    /* tab released */
                case 0x10: return 'Q';  // Q pressed
                case 0x11: return 'W';
                case 0x12: return 'E';
                case 0x13: return 'R';
                case 0x14: return 'T';
                case 0x15: return 'Y';
                case 0x16: return 'U';
                case 0x17: return 'I';
                case 0x18: return 'O';
                case 0x19: return 'P';
                case 0x1A: return '{';
                case 0x1B: return '}';
                case 0x2B: return '|';

                // second row
                case 0x3A: caps ^= 1; return 0x00;      /* toggle caps lock */
                case 0x1E: return 'A';
                case 0x1F: return 'S';
                case 0x20: return 'D';
                case 0x21: return 'F';
                case 0x22: return 'G';
                case 0x23: return 'H';
                case 0x24: return 'J';
                case 0x25: return 'K';
                case 0x26: return 'L';
                case 0x27: return ':';
                case 0x28: return '\"';
                /* enter should be here - 0x1C */
                case 0x1C: return '\n';

                // third row
                case 0x2A: l_shift = 1; return 0x00;    /* left shift pressed */
                case 0xAA: l_shift = 0; return 0x00;    /* left shift released */
                case 0x2C: return 'Z';
                case 0x2D: return 'X';
                case 0x2E: return 'C';
                case 0x2F: return 'V';
                case 0x30: return 'B';
                case 0x31: return 'N';
                case 0x32: return 'M';
                case 0x33: return '<';
                case 0x34: return '>';
                case 0x35: return '?';
                case 0x36: r_shift = 1; return 0x00;    /* right shift pressed */
                case 0xB6: r_shift = 0; return 0x00;    /* right shift released */

                // fourth row
                case 0x1D: ctrl = 1; return 0x00;     /* left/right control pressed */
                case 0x9D: ctrl = 0; return 0x00;     /* left/right control released */
                case 0x38: alt = 1; return 0x00;      /* left/right alt pressed */
                case 0xB8: alt = 0; return 0x00;      /* left/right alt released */
                case 0x39: return ' ';

                // function keys pressed
                case 0x3B: function = F1; return 0x00;
                case 0x3C: function = F2; return 0x00;
                case 0x3D: function = F3; return 0x00;
                // function key unpressed
                case 0xBB: function = 0; return 0x00;
                case 0xBC: function = 0; return 0x00;
                case 0xBD: function = 0; return 0x00;

                default: return 0x00;
            }

        // Shift with capslock toggled
        case 3:
            switch(keypress) {
                // numbers pressed
                case 0x29: return '~';  // ` pressed
                case 0x02: return '!';  // 1 pressed
                case 0x03: return '@';  // 2 pressed
                case 0x04: return '#';  // 3 pressed
                case 0x05: return '$';  // 4 pressed
                case 0x06: return '%';  // 5 pressed
                case 0x07: return '^';  // 6 pressed
                case 0x08: return '&';  // 7 pressed
                case 0x09: return '*';  // 8 pressed
                case 0x0A: return '(';  // 9 pressed
                case 0x0B: return ')';  // 0 pressed
                case 0x0C: return '_';  // - pressed
                case 0x0D: return '+';  // = pressed
                /* backspace should be here - 0x0E */
                case 0x0E: backspace = 1; return 0x00;  /* backspace pressed */
                case 0x8E: backspace = 0; return 0x00;  /* backspace released */

                // first row of letters
                case 0x0F: tab = 1; return 0x00;    /* tab pressed */
                case 0x8F: tab = 0; return 0x00;    /* tab released */
                case 0x10: return 'q';  // Q pressed
                case 0x11: return 'w';
                case 0x12: return 'e';
                case 0x13: return 'r';
                case 0x14: return 't';
                case 0x15: return 'y';
                case 0x16: return 'u';
                case 0x17: return 'i';
                case 0x18: return 'o';
                case 0x19: return 'p';
                case 0x1A: return '[';
                case 0x1B: return ']';
                case 0x2B: return '\\';

                // second row
                case 0x3A: caps ^= 1; return 0x00;      /* toggle caps lock */
                case 0x1E: return 'a';
                case 0x1F: return 's';
                case 0x20: return 'd';
                case 0x21: return 'f';
                case 0x22: return 'g';
                case 0x23: return 'h';
                case 0x24: return 'j';
                case 0x25: return 'k';
                case 0x26: return 'l';
                case 0x27: return ';';
                case 0x28: return '\'';
                /* enter should be here - 0x1C */
                case 0x1C: return '\n';

                // third row
                case 0x2A: l_shift = 1; return 0x00;    /* left shift pressed */
                case 0xAA: l_shift = 0; return 0x00;    /* left shift released */
                case 0x2C: return 'z';
                case 0x2D: return 'x';
                case 0x2E: return 'c';
                case 0x2F: return 'v';
                case 0x30: return 'b';
                case 0x31: return 'n';
                case 0x32: return 'm';
                case 0x33: return ',';
                case 0x34: return '.';
                case 0x35: return '/';
                case 0x36: r_shift = 1; return 0x00;    /* right shift pressed */
                case 0xB6: r_shift = 0; return 0x00;    /* right shift released */

                // fourth row
                case 0x1D: ctrl = 1; return 0x00;     /* left/right control pressed */
                case 0x9D: ctrl = 0; return 0x00;     /* left/right control released */
                case 0x38: alt = 1; return 0x00;      /* left/right alt pressed */
                case 0xB8: alt = 0; return 0x00;      /* left/right alt released */
                case 0x39: return ' ';

                // function keys pressed
                case 0x3B: function = F1; return 0x00;
                case 0x3C: function = F2; return 0x00;
                case 0x3D: function = F3; return 0x00;
                // function key unpressed
                case 0xBB: function = 0; return 0x00;
                case 0xBC: function = 0; return 0x00;
                case 0xBD: function = 0; return 0x00;

                default: return 0x00;
            }
    }

    return 0x00;
}

