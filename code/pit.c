#include "pit.h"
#include "keyboard.h"

int terminal_init = 1;
int exec_ready = 0;

/* void pit_init(void)
 * Inputs: void
 * Return Value: void
 * Function: Initializes PIT to IRQ0 */
void pit_init() {
    int* flags;

    cli_and_save(flags);

    outb(MODE_4_SELECT, MODE_REG_PORT);     // set mode

    // write reload value to get interrupts every INT_TIME_MS ms
    outb(PIT_REL_VAL & LOW_BYTE_MASK, CHANNEL0_PORT);
    outb((PIT_REL_VAL >> HIGH_BYTE_SHIFT) & LOW_BYTE_MASK, CHANNEL0_PORT);

    enable_irq(0);

    restore_flags(flags);
    sti();
}

/* void pit_handler(void)
 * Inputs: void
 * Return Value: void
 * Function: Switches active terminal process to be execute */
void pit_handler() {
    cli();
    
    // get old pcb
    int active_pid = terminals[ta].active_pid;
    pcb_t* cur_pcb;
    cur_pcb = (pcb_t*)( CONST_8MB -((active_pid + 1) * CONST_8KB) );
    
    // save current process ebp & esp into pcb
    uint32_t k_esp;
    asm volatile("movl %%esp, %0" : "=r"(k_esp) : : "memory");
    cur_pcb->cur_esp = k_esp;
    uint32_t k_ebp;
    asm volatile("movl %%ebp, %0" : "=r"(k_ebp) : : "memory");
    cur_pcb->cur_ebp = k_ebp;

    if (terminal_init < TOTAL_TERM) {
        terminal_switch(terminal_init+1);
        set_ta(terminal_init);

        terminals[terminal_init].base_pid = terminal_init;
        terminals[terminal_init].active_pid = terminal_init;
        
        terminal_init++;

        send_eoi(0);

        sys_execute((const uint8_t*)"shell");

        sti();

        return;
    }
    
    else {
        if(terminal_init == TOTAL_TERM){
            terminal_switch(1);     // return to first terminal
            term_init_done = 1;
            terminal_init++;
        }
        send_eoi(0);
    
        // change active terminal
        int active_term = (get_active_terminal() + 1) % TOTAL_TERM;
        ta = active_term;

        // switch to active_term's process
        active_pid = terminals[ta].active_pid;

        pcb_t* active_pcb;
        active_pcb = (pcb_t*)( CONST_8MB -((active_pid + 1) * CONST_8KB) );

        // update global pids
        set_cur_pid(active_pid);
        set_prev_pid(active_pcb->parent_pid);

        // update program image page
        uint32_t pid_phys_addr = (active_pid+2) << OFFSET_10;
        pd[PROGRAM_IMG_IDX].p = 1;
        pd[PROGRAM_IMG_IDX].addr = pid_phys_addr;

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

        tss.esp0 = CONST_8MB - (active_pid*CONST_8KB) - sizeof(uint32_t);

        asm volatile(
            "movl %0, %%ebp;"
            "movl %1, %%esp;"
            :
            :"r"(active_pcb->cur_ebp), "r"(active_pcb->cur_esp)
            :"memory"
        );
        sti();

        return;
    }

    send_eoi(0);
    sti();
}
