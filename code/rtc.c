#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "idt.h"
#include "lib.h"
#include "system_call.h"
#include "processes.h"

int rtc_wait = 0;
int count_rtc = 1;
int shell_freq = 2;
int process_rtc[MAX_PID];

int terminal_rtc_count[NUM_TERMS] = {0, 0, 0};
int terminal_rtc_wait[NUM_TERMS] = {0, 0, 0};

/* void rtc_init(void)
 * Inputs: void
 * Return Value: void
 * Function: Initialize the RTC by enabling the IRQ8 line to the PIC */
void rtc_init() {
    int* flags;

    cli_and_save(flags);                // disable interrupts and save flags

    // Turning on periodic interrupt
    outb(REG_B, NMI);		    // select register B, and disable NMI
    char prev=inb(RW);	    // read the current value of register B
    outb(REG_B, NMI);		    // set the index again (a read will reset the index to register D)
    outb(prev | BIT_6_MASK, RW);	// write the previous value ORed with 0x40. This turns on bit 6 of register B

    enable_irq(RTC_IRQ);        // Enable IRQ8

    outb(REG_A, NMI);                   // select register A, disable NMI
    prev = inb(RW);              // get initial value of register A
    outb(REG_A, NMI);                   // reset index to A
    outb((prev & TOP_4_MASK) | MIN_VIR_RATE, RW);   // write rate to A

    int i;
    for(i = 0; i < MAX_PID; i++){
        process_rtc[i] = MAX_VIR_FREQ;  // initialize all rtc to 1024Hz
    }

    restore_flags(flags);               // restore flags
    sti();                              // enable interrupts
}


/* void rtc_handler(void)
 * Inputs: void
 * Return Value: void
 * Function: Handles the RTC interrupts  */
void rtc_handler(void) {
    cli();

    int i;
    int cur_pid;
    int cur_freq;

    count_rtc++;

    // increment each terminal's rtc count
    for (i = 0; i < NUM_TERMS; i++) {
        cur_pid = terminals[i].active_pid;
        cur_freq = process_rtc[cur_pid];

        terminal_rtc_count[i]++;

        // check each terminal's wait
        if( (terminal_rtc_count[i] % (MAX_VIR_FREQ/cur_freq) == 0) ) {
            terminal_rtc_wait[i] = 0;
        }
    }

    // Flicker every half second
    if(count_rtc % MAX_VIR_FREQ == 0){   
        put_cursor();
    }
    if(count_rtc % (MAX_VIR_FREQ/2) == 0 && count_rtc % (MAX_VIR_FREQ) != 0) {
        rem_cursor();
    }
    
    // Ensuring continuous interrupts
    outb(REG_C, NMI);	// select register C
    inb(RW);		    // just throw away contents

    send_eoi(IRQ8);
    sti();
}


/* int rtc_open(void)
 * Inputs: void
 * Return Value: int
 * Function: Initializes the RTC frequency to 2 Hz */
int32_t rtc_open(uint8_t* file_name) {
    int cur_pid = get_cur_pid();
    process_rtc[cur_pid] = MIN_VIR_FREQ;
    return 0;
}


/* int rtc_close(void)
 * Inputs: void
 * Return Value: int
 * Function: Does nothing until we virtualize in the future */
int32_t rtc_close(uint8_t* file_name) {
    int cur_pid = get_cur_pid();
    process_rtc[cur_pid] = MAX_VIR_FREQ;
    return 0;
}


/* int rtc_read(void)
 * Inputs: void
 * Return Value: int
 * Function: Should block until the next interrupt */
int32_t rtc_read(int32_t fd, uint8_t* buf, int32_t nbytes) {
    int active_term = get_active_terminal();

    terminal_rtc_wait[active_term] = 1;

    while(terminal_rtc_wait[active_term] == 1) {}

    return 0;
}


/* int rtc_write(void)
 * Inputs: void
 * Return Value: int
 * Function: Must be able to change the frequency */
int32_t rtc_write(int32_t fd, uint8_t* buf, int32_t rate) {
    cli();
    
    int32_t freq = *(uint32_t*)buf;
    if (freq < MIN_VIR_FREQ || freq > MAX_VIR_FREQ) {return -1;}

    int cur_pid = get_cur_pid();

    process_rtc[cur_pid] = freq;

    sti();

    return 0;
}


/* void set_rtc_wait(void)
 * Inputs: void
 * Return Value: void
 * Function: Set the rtc_wait flag to break the rtc_read block */
void set_rtc_wait(){
    rtc_wait = 0;
}
