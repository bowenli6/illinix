#include <types.h>
#include <boot/idt.h>
#include <boot/x86_desc.h>
#include <boot/exception.h>
#include <boot/interrupt.h>
#include <boot/syscall.h>
#include <lib.h>
#include <io.h>



/* Local functions, see headers for descriptions. */

static void ignore_int_handler();
static void set_trap_gate(uint8_t n, void (*handler)());
static void set_intr_gate(uint8_t n, void (*handler)());
static void set_system_gate(uint8_t n, void (*handler)());
static void set_system_intr_gate(uint8_t n, void (*handler)());
// static void set_task_gate(uint8_t n, uint8_t gdt);


/*
 * Reference: MP3 Appendix-D
 * Hardware interrupt handlers and exception handlers should have their DPL set to 0 to
 * prevent userlevel applications from calling into these routines with the int instruction.
 *
 * The system call handler should have its DPL set to 3 so that it is accessible from userSpace
 * via the int instruction.
 *
 * Finally, each IDT entry also contains a segment selector field that specifies a code segment
 * in the GDT, and you should set this field to be the kernel’s code segment descriptor.
 */
/*
 * Recerence: to IA32 5-24 Figure 5-2.
 * Exception/Trap: DLP(0), 0D111
 * Interrupt: DLP(0), 0D110
 */


/**
 * @brief Initialization of the IDT.
 * Filling all 256 entries of idt table
 * with the same interrupt gate, which
 * refers to the ignore_init_handler()
 * interrupt handler.
 */
void idt_init() {
    int i;
    for (i = 0; i < IDT_SIZE; i++) {
        /* Set up each IDT descriptor value.
         * All init to interrupt gate. 
         */
        idt[i].seg_selector = KERNEL_CS;      
        idt[i].present = 1;    
        idt[i].dpl = 0;             
        idt[i].reserved0 = 0;    
        idt[i].size = 1;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved3 = 0;       /* 0 if interrupt gate, 1 if trap gate. */
        idt[i].reserved4 = 0;
        SET_IDT_ENTRY(idt[i], &ignore_int_handler); 
    }
}


/**
 * @brief Initialized with an exception
 * handler function for each recognized
 * exception. Insert the handler into
 * all IDT entries that refer to nonmaskable
 * interrupts and exceptions.
 */
void trap_init() {
    set_trap_gate(DIVIDE_ERROR, &divide_error_handler);
    set_trap_gate(DEBUG, &debug_handler);
    set_intr_gate(NMI, &nmi_handler);
    set_system_intr_gate(BREAKPOINT, &int3_handler);
    set_system_gate(OVERFLOW, &overflow_handler);
    set_system_gate(BOUNDS_CHECK, &bounds_handler);
    set_trap_gate(INVALID_OPCODE, &invalid_op_handler);
    set_trap_gate(DEVICE_NOT_AVAILIAVLE, &device_not_available_handler);
    set_trap_gate(DOUBLE_FAULT, &double_fault_handler);
    set_trap_gate(COPROCESSOR_OVERRUN, &coprocessor_segment_overrun_handler);
    set_trap_gate(INVALID_TSS, &invalid_TSS_handler);
    set_trap_gate(SEGMENT_NOT_PRESENT, &segment_not_present_handler);
    set_trap_gate(STACK_SEGMENT_FAULT, &stack_segment_handler);
    set_trap_gate(GENRAL_PROTECTION, &general_protection_handler);
    set_trap_gate(PAGE_FAULT, &page_fault_handler);
    set_trap_gate(FLOATING_POINT_ERROR, &coprocessor_error_handler);
    set_trap_gate(ALIGHMENT_CHECK, &alignment_check_handler);
    set_trap_gate(MACHINE_CHECK, &machine_check_handler);
    set_trap_gate(SIMD_FLOATING_POINT, &simd_coprocessor_error_handler);
    set_system_gate(SYSCALL, &syscall_handler);
}


/**
 * @brief Initialize interrupt handlers 
 * from device drivers.
 */
void intr_init() {
    set_intr_gate(TIMER_INTR, &timer_handler);
    set_intr_gate(KEYBOARD_INTR, &keyboard_handler);
    set_intr_gate(RTC_INTR, &rtc_handler);
}



/**
 * @brief Insterts a trap gate in the nth IDT entry 
 * for an exception handler.
 * 
 * @param n : The nth IDT entry to be set.
 * @param handler : A exception handler.
 */
static void set_trap_gate(uint8_t n, void (*handler)()) {
    idt[n].seg_selector = KERNEL_CS;
    idt[n].dpl = 0;
    idt[n].reserved3 = 1;
    SET_IDT_ENTRY(idt[n], handler);
}


/**
 * @brief Insterts a interrupt gate in the nth IDT entry 
 * for an interrupt handler.
 * 
 * @param n : The nth IDT entry to be set.
 * @param handler : A interrupt handler.
 */
static void set_intr_gate(uint8_t n, void (*handler)()) {
    idt[n].seg_selector = KERNEL_CS;
    idt[n].dpl = 0;
    SET_IDT_ENTRY(idt[n], handler);
}


/**
 * @brief Insterts a system gate in the nth IDT entry 
 * for an exception handler.
 * 
 * @param n : The nth IDT entry to be set.
 * @param handler : A exception handler.
 */
static void set_system_gate(uint8_t n, void (*handler)()) {
    idt[n].seg_selector = KERNEL_CS;
    idt[n].dpl = 3;
    idt[n].reserved3 = 1;
    SET_IDT_ENTRY(idt[n], handler);
}


/**
 * @brief Insterts a system interrupt gate in the nth IDT entry 
 * for an interrupt handler.
 * 
 * @param n : The nth IDT entry to be set.
 * @param handler : A interrupt handler.
 */
static void set_system_intr_gate(uint8_t n, void (*handler)()) {
    idt[n].seg_selector = KERNEL_CS;
    idt[n].dpl = 3;
    SET_IDT_ENTRY(idt[n], handler);
}


/**
 * @brief Insterts a task gate in the nth IDT entry. 
 * Include the TSS selector of the process that must
 * replace the current one when an interrupt signal
 * occurs.
 * 
 * @param n : The nth IDT entry to be set.
 * @param gdt : The index in the GDT of the TSS 
 * containing the function to be activated.
 */
// static void set_task_gate(uint8_t n, uint8_t gdt) {
//     idt[n].seg_selector = gdt;
//     idt[n].dpl = 3;    
//     SET_IDT_ENTRY(idt[n], 0);
// }


/**
 * @brief A unknown interrupt handler.
 * 
 */
static void ignore_int_handler() {
    uint32_t interrupt_flag;
    cli_and_save(interrupt_flag);
    printf("Unknown interrupt.\n");
    restore_flags(interrupt_flag);
}
