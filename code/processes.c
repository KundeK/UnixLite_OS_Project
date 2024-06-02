#include "processes.h"

/* uint32_t process_init()
 * Inputs:
 * Return Value: 0
 * Function: Initializes fops tables with correct functions */
uint32_t process_init() {
    file_opt.open = file_open;
    file_opt.close = file_close;
    file_opt.read = file_read;
    file_opt.write = file_write;

    dir_opt.open = dir_open;
    dir_opt.close = dir_close;
    dir_opt.read = dir_read;
    dir_opt.write = dir_write;
    
    rtc_opt.open = rtc_open;
    rtc_opt.close = rtc_close;
    rtc_opt.read = rtc_read;
    rtc_opt.write = rtc_write;
    
    terminal_opt.open = terminal_open;
    terminal_opt.close = terminal_close;
    terminal_opt.read = terminal_read;
    terminal_opt.write = terminal_write;
    
    stdin_opt.open = terminal_open;
    stdin_opt.close = terminal_close;
    stdin_opt.read = terminal_read;
    stdin_opt.write = invalid_read_write;
    
    stdout_opt.open = terminal_open;
    stdout_opt.close = terminal_close;
    stdout_opt.read = invalid_read_write;
    stdout_opt.write = terminal_write;

    null_opt.open = invalid_open_close;
    null_opt.close = invalid_open_close;
    null_opt.read = invalid_read_write;
    null_opt.write = invalid_read_write;

    terminals[0].cursor_y = CURSOR_Y_START;
    terminals[0].cursor_x = CURSOR_X_START;

    return 0;
}

/* int32_t invalid_open_close(uint8_t* file_name)
 * Inputs: file_name - boiler plate parameter to match system call
 * Return Value: -1 for failure
 * Function: function to represent invalid open/close system call */
int32_t invalid_open_close(uint8_t* file_name) {
    return -1;
}

/* int32_t invalid_read_write(int32_t fd, uint8_t* buf, int32_t nbytes)
 * Inputs: fd, buf, nbytes - boiler plate parameter to match system call
 * Return Value: -1 for failure
 * Function: function to represent invalid read/write system call */
int32_t invalid_read_write(int32_t fd, uint8_t* buf, int32_t nbytes) {
    return -1;
}

/* int32_t set_active_terminal(int active_term)
 * Inputs: int active_term - active terminal to switch to
 * Return Value: -1 for failure, 0 for success
 * Function: moves the active terminal to the next */
int32_t set_active_terminal(int active_term) {
    if (active_term < 0 || active_term >= TOTAL_TERM) {return -1;}   // check valid terminal

    int active_pid = terminals[ta].active_pid;

    // save current process ebp & esp
    pcb_t* cur_pcb;
    cur_pcb = (pcb_t*)( CONST_8MB -((active_pid + 1) * CONST_8KB) );

    uint32_t k_esp;
    asm volatile("movl %%esp, %0" : "=r"(k_esp) : : "memory");
    uint32_t k_ebp;
    asm volatile("movl %%ebp, %0" : "=r"(k_ebp) : : "memory");
    
    cur_pcb->cur_ebp = k_ebp;
    cur_pcb->cur_esp = k_esp;

    // change active terminal
    ta = active_term;

    // switch to active_term's process
    active_pid = terminals[ta].active_pid;

    printf("Switching to pid %d", active_pid);

    pcb_t* active_pcb;
    active_pcb = (pcb_t*)( CONST_8MB -((active_pid + 1) * CONST_8KB) );

    // update global pids
    set_cur_pid(active_pid);
    set_prev_pid(active_pcb->parent_pid);

    // update program image page
    uint32_t pid_phys_addr = (active_pid+2) << OFFSET_10;
    pd[PROGRAM_IMG_IDX].p = 1;
    pd[PROGRAM_IMG_IDX].addr = pid_phys_addr;

    // flush TLB
    asm volatile("movl %cr3, %eax;"
                 "movl %eax, %cr3");

    tss.esp0 = active_pcb->cur_esp;

    asm volatile(
        "movl %0, %%ebp;"
        "movl %1, %%esp;"
        :
        :"r"(active_pcb->cur_ebp), "r"(active_pcb->cur_esp)
        :"memory"
    );

    return 0;
}

/* int get_active_terminal()
 * Inputs: void
 * Return Value: returns active terminal
 * Function: getter for ta variable */
int get_active_terminal() {
    return ta;
}

int get_shown_terminal() {
    return ts;
}

/* int set_ta(int new_ta)
 * Inputs: int new_ta - terminal to switch to active
 * Return Value: void
 * Function: setter for ta variable */
void set_ta(int new_ta) {
    if (new_ta < 0 || new_ta > 2) {return;}
    ta = new_ta;
}
