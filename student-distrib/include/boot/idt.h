#ifndef _IDT_H
#define _IDT_H

#define IDT_SIZE                    256

void idt_init();
void trap_init();
void intr_init();

#endif


