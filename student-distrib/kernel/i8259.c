/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include <boot/i8259.h>

#include <lib.h>

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint16_t irq_mask = 0xffff;                 /* IRQs 0 ~ 15 */

/**
 * @brief Initialize the 8259 PIC.
 */
void i8259_init() {
	outb(0xff, PIC_MASTER_IMR);	            /* mask all of 8259A-1 */
	outb(0xff, PIC_SLAVE_IMR);	            /* mask all of 8259A-2 */

	outb(ICW1, PIC_MASTER_CMD);	            /* ICW1: select 8259A-1 init */
	outb(ICW2_MASTER, PIC_MASTER_IMR);	    /* ICW2: 8259A-1 IR0-7 mapped to 0x20-0x27 */
	outb(ICW3_MASTER, PIC_MASTER_IMR);	    /* 8259A-1 (the master) has a slave on IR2 */
	outb(ICW4, PIC_MASTER_IMR);             /* master expects normal EOI */

	outb(ICW1, PIC_SLAVE_CMD);	            /* ICW1: select 8259A-2 init */
    outb(ICW2_SLAVE, PIC_SLAVE_IMR);        /* ICW2: 8259A-2 IR0-7 mapped to 0x28-0x2f */
	outb(ICW3_SLAVE, PIC_SLAVE_IMR);	    /* 8259A-2 is a slave on master's IR2 */
	outb(ICW4, PIC_SLAVE_IMR);	            /* slave expects normal EOI */

    outb(master_mask, PIC_MASTER_IMR);      /* restore master IRQ mask */
	outb(slave_mask, PIC_SLAVE_IMR);	    /* restore slave IRQ mask */

    enable_irq(PIC_SLAVE_PORT);		    	/* Enable IRQ2 from Master to connect with slave 8259A-2. */
}

/**
 * @brief Enable (unmask) the specified IRQ.
 * 
 * @irq_num: The number of mask should be set to 0
 * 
 */
void enable_irq(uint32_t irq_num) {
    unsigned int mask = ~(1 << irq_num);            
	irq_mask &= mask;                               /* enable the mask. */
	if (irq_num & PIC_PORT_TOTAL)                                
		outb(slave_mask, PIC_SLAVE_IMR);			
	else
		outb(master_mask, PIC_MASTER_IMR);
}

/**
 * @brief Disable (mask) the specified IRQ.
 * 
 * @irq_num: The number of mask should be set to 1
 * 
 */
void disable_irq(uint32_t irq_num) {
    unsigned int mask = 1 << irq_num;
	irq_mask |= mask;                               /* disable the mask. */
	if (irq_num & PIC_PORT_TOTAL)
		outb(slave_mask, PIC_SLAVE_IMR);
	else
		outb(master_mask, PIC_MASTER_IMR);
}

/**
 * @brief Send end-of-interrupt signal for the specified IRQ
 * 
 * @irq_num: The number of interrupt that has just done.
 * 
 */
void send_eoi(uint32_t irq_num) {
	uint8_t eoi = EOI;								
	if (irq_num & PIC_PORT_TOTAL) {
		eoi |= (irq_num - PIC_PORT_TOTAL);			/* This gets OR'd with the interrupt number. */
		outb(eoi, PIC_SLAVE_CMD);					/* Send eoi out to the Slave PIC. */
		send_eoi(PIC_SLAVE_PORT);					/* Also send send eoi to the MASTER's IRQ2. */
	} else {
		eoi |= irq_num;								/* This gets OR'd with the interrupt number. */
		outb(eoi, PIC_MASTER_CMD);					/* Send eoi out to the Master PIC. */
	}
}
