#ifndef _IDT_H
#define _IDT_H

#include "exception.h"
#include "interrupt.h"
#include "syscall.h"

#define IDT_SIZE                    256

void idt_init();
void trap_init();
void ignore_int_handler();

#endif


