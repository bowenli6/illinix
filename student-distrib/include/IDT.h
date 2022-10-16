#ifndef _IDT_H
#define _IDT_H

#include "../x86_desc.h"
#include "exception.h"
#include "interrupt.h"

void idt_init();

/**
 * @brief Insterts a trap gate in the nth IDT entry for an exception handler.
 * 
 * @param n : The nth IDT entry to be set.
 * @param handler : A exception handler.
 */
void set_trap_gate(uint8_t n, void (*handler)());

/**
 * @brief Insterts a interrupt gate in the nth IDT entry for an interrupt handler.
 * 
 * @param n : The nth IDT entry to be set.
 * @param handler : A interrupt handler.
 */
void set_intr_gate(uint8_t n, void (*handler)());

/**
 * @brief Insterts a task gate in the nth IDT entry for an system call. 
 * 
 */
void set_task_gate(uint8_t n, )

#endif


