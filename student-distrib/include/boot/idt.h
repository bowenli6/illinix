#ifndef _IDT_H
#define _IDT_H

#include <boot/exception.h>
#include <boot/interrupt.h>
#include <boot/syscall.h>

#define IDT_SIZE                    256

void idt_init();
void trap_init();
void intr_init();
void ignore_int_handler();

#endif


