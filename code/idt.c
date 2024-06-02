#include "idt.h"
#include "lib.h"            
#include "x86_desc.h"      
#include "exception.h"
#include "system_call.h"
#include "i8259.h"
#include "rtc.h"
#include "keyboard.h"
#include "terminal.h"
#include "keyboard_link.h"
#include "rtc_link.h"
#include "pit.h"
#include "pit_link.h"

// Exception messages for display purposes
static char* exception_messages[] = {
    "Division Error", "Debug Exception", "Non-Maskable Interrupt", "Breakpoint Exception",
    "Overflow", "Bounds Range Exceeded", "Invalid Opcode", "Device Not Available",
    "Double Fault", "Coprocessor Segment Overrun", "Invalid TSS", "Segment Not Present",
    "Stack Fault", "General Protection Fault", "Page Fault", "Reserved",
    "x87 Floating-Point Exception", "Alignment Check", "Machine Check",
    "SIMD Floating-Point Exception"
};
// Global variables for keyboard and RTC



/* void initialize_idt()
 * Inputs: void
 * Return Value: void
 * Function: Initialize the IDT by setting the struct values of the IDT array for all of the handlers */
void initialize_idt() {
    unsigned int i;
    for (i = 0; i < NUM_VEC; i++) {
        idt[i].seg_selector = KERNEL_CS;
        
        if (i == SYSCALL_VEC_NUM) {
            idt[i].dpl = 0x3; // DPL 3 for syscalls
        } 
        else {
            idt[i].dpl = 0x0;
        }

        if ( (i < NUM_EXCEPTIONS || i == SYSCALL_VEC_NUM || i == KEYBOARD_VEC_NUM || i == RTC_VEC_NUM || i == PIT_VEC_NUM) && i != INTEL_RESERVED) {
            idt[i].present = 1;
        } 
        else {
            idt[i].present = 0;
        }

        idt[i].reserved0 = 0;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        
        if (i >= NUM_EXCEPTIONS) {
            idt[i].reserved3 = 1; // Interrupt gates for IRQs
        } 
        else {
            idt[i].reserved3 = 0;
        }

        idt[i].reserved4 = 0;
        idt[i].size = 1; // 32-bit interrupt gate
    }

    // Set handlers for exceptions
    SET_IDT_ENTRY(idt[0], divide_error);
    SET_IDT_ENTRY(idt[1], single_step);
    SET_IDT_ENTRY(idt[2], nmi);
    SET_IDT_ENTRY(idt[3], breakpoint);
    SET_IDT_ENTRY(idt[4], overflow);
    SET_IDT_ENTRY(idt[5], bound_range_exceeded);
    SET_IDT_ENTRY(idt[6], invalid_opcode);
    SET_IDT_ENTRY(idt[7], dna);
    SET_IDT_ENTRY(idt[8], double_fault);
    SET_IDT_ENTRY(idt[9], coproc);
    SET_IDT_ENTRY(idt[10], invalid_tss);
    SET_IDT_ENTRY(idt[11], snp);
    SET_IDT_ENTRY(idt[12], ssf);
    SET_IDT_ENTRY(idt[13], gen_prot);
    SET_IDT_ENTRY(idt[14], page_fault);
    SET_IDT_ENTRY(idt[16], math_fault);
    SET_IDT_ENTRY(idt[17], align_check);
    SET_IDT_ENTRY(idt[18], machine_check);
    SET_IDT_ENTRY(idt[19], floatp_excep);

    // Device specific handlers
    SET_IDT_ENTRY(idt[KEYBOARD_VEC_NUM], keyboard);
    SET_IDT_ENTRY(idt[RTC_VEC_NUM], rtc);
    SET_IDT_ENTRY(idt[PIT_VEC_NUM], pit);

    // System call
    SET_IDT_ENTRY(idt[SYSCALL_VEC_NUM], sys_call);

    // Loading the IDT
    lidt(idt_desc_ptr);
}

/* void handle_exception(uint32_t id, uint32_t err_code)
 * Inputs: uint32_t id, uint32_t err_code
 * Return Value: void
 * Function: Handle any exceptions thrown by the code by displaying the exception message and halting */
void handle_exception(uint32_t id, uint32_t err_code) {
    cli();

    clear_screen();

    if(err_code == NO_ERR_CODE) {
        printf("EXCEPTION DETECTED      Exception: %s\n\n", exception_messages[id]);
    } else {
        printf("EXCEPTION DETECTED      Exception: %s - Error Code: 0x%x\n",exception_messages[id], err_code);
    }

    // while(1){};
    halt_helper(EXCEPTION_HALT);

    return;
}
