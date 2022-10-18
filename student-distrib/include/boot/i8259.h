/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _I8259_H
#define _I8259_H

#include <types.h>

/* Ports that each PIC sits on */
#define PIC_PORT_TOTAL    8
#define PIC_MASTER_CMD    0x20
#define PIC_SLAVE_CMD     0xA0

#define PIC_MASTER_IMR    0x21
#define PIC_SLAVE_IMR     0xA1

#define PIC_SLAVE_PORT    0x02      /* The SLAVE is connected to the IRQ2 of MASTER. */

/* Initialization control words to init each PIC.
 * See the Intel manuals for details on the meaning
 * of each word */
#define ICW1                0x11
#define ICW2_MASTER         0x20
#define ICW2_SLAVE          0x28
#define ICW3_MASTER         0x04
#define ICW3_SLAVE          0x02
#define ICW4                0x01

extern uint16_t irq_mask;                           
#define __byte(x,y) (((unsigned char *) &(y))[x])   /* Get the xth bytes of y. */
#define master_mask	(__byte(0, irq_mask))           /* The first byte of irq_mask is master_mask. */
#define slave_mask	(__byte(1, irq_mask))           /* The second byte of the irq_mask is slave_mask. */

/* End-of-interrupt byte.  This gets OR'd with
 * the interrupt number and sent out to the PIC
 * to declare the interrupt finished */
#define EOI                 0x60

/* Externally-visible functions */

void i8259_init(void);
void enable_irq(uint32_t irq_num);
void disable_irq(uint32_t irq_num);
extern void send_eoi(uint32_t irq_num);

#endif /* _I8259_H */

