/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = MASK_ALL; /* IRQs 0-7  */
uint8_t slave_mask = MASK_ALL;  /* IRQs 8-15 */

/* void i8259_init(void)
 * Inputs: void
 * Return Value: void
 * Function: Initialize the PICs by sending the control words to the master and then the slave PICs */
void i8259_init(void) {
    outb(ICW1, MASTER_8259_PORT);                   // ICW1
    outb(ICW2_MASTER, MASTER_8259_DATA_PORT);       // ICW2 - Master PIC
    outb(ICW3_MASTER, MASTER_8259_DATA_PORT);       // ICW3 - Master PIC
    outb(ICW4, MASTER_8259_DATA_PORT);              // ICW4 - Master PIC

    outb(ICW1, SLAVE_8259_PORT);                    // ICW1
    outb(ICW2_SLAVE, SLAVE_8259_DATA_PORT);         // ICW2 - Slave PIC
    outb(ICW3_SLAVE, SLAVE_8259_DATA_PORT);         // ICW3 - Slave PIC
    outb(ICW4, SLAVE_8259_DATA_PORT);               // ICW4 - Slave PIC

    outb(MASK_ALL, MASTER_8259_DATA_PORT);          // mask all of Master PIC
    outb(MASK_ALL, SLAVE_8259_DATA_PORT);           // mask all of Slave PIC

    enable_irq(ICW3_SLAVE);         // enable secondary pic
}

/* void enable_irq(uint32_t irq_num)
 * Inputs: uint32_t irq_num
 * Return Value: void
 * Function: Enables the specific IRQ on the PICs */
void enable_irq(uint32_t irq_num) {
    if (irq_num >= PIC_IRQ_NUM*2) {return;}

    if (irq_num < PIC_IRQ_NUM) {
        uint8_t mask = ~(0x01 << irq_num);          // shift left to correct irq to unmask
        master_mask &= mask;                        // unmasks specified irq on master pic

        outb(master_mask, MASTER_8259_DATA_PORT);   // send mask data to master pic
    }
    else {
        uint8_t mask = ~(0x01 << (irq_num-PIC_IRQ_NUM));      // shift left to correct irq on slave to unmask
        slave_mask &= mask;                         // unmasks specified irq on slave pic

        outb(slave_mask, SLAVE_8259_DATA_PORT);     // send mask data to slave pic
    }
}

/* void disable_irq(uint32_t irq_num)
 * Inputs: uint32_t irq_num
 * Return Value: void
 * Function: Disabling the specific IRQ on the PICs */
void disable_irq(uint32_t irq_num) {
    if (irq_num >= PIC_IRQ_NUM*2) {return;}

    if (irq_num < PIC_IRQ_NUM) {
        uint8_t mask = 0x01 << irq_num;             // shift left to correct irq to mask
        master_mask |= mask;                        // masks specified irq on master pic

        outb(master_mask, MASTER_8259_DATA_PORT);   // send mask data to master pic
    }
    else {
        uint8_t mask = 0x01 << (irq_num-PIC_IRQ_NUM);         // shift left to correct irq on slave to mask
        slave_mask |= mask;                         // masks specified irq on slave pic

        outb(slave_mask, SLAVE_8259_DATA_PORT);     // send mask data to slave pic
    }
}

/* void send_eoi(uint32_t irq_num)
 * Inputs: uint32_t irq_num
 * Return Value: void
 * Function: Sends the end of interrupt signal after handling has been completed */
void send_eoi(uint32_t irq_num) {
    if (irq_num >= PIC_IRQ_NUM*2) {return;}

    if (irq_num >= PIC_IRQ_NUM) {
        uint8_t pic2_irq = irq_num - PIC_IRQ_NUM;       // slave PIC irq number
        uint8_t pic1_irq = ICW3_SLAVE;                  // master PIC irq number

        outb(EOI | pic2_irq, SLAVE_8259_PORT);     // send EOI to slave PIC
        outb(EOI | pic1_irq, MASTER_8259_PORT);    // send EOI to master PIC
    }
    else {
        outb(EOI | irq_num, MASTER_8259_PORT);     // send EOI to master PIC
    }
}
