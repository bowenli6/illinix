#include "../include/exception.h"
#include "../lib/lib.h"


/* According to IA32 page 6, we define the name for first 20 exceptions */
static const char *exception_arr[EXCEPTION_COUNT] = {
    "DE: Divide Error Exception",
    "DB: Debug Exception",
    "NMI: Non-maskable Interrupt",
    "BP: Breakpoint Exception",
    "OF: Overflow Exceptioin",
    "BR: BOUND Range Exceeded Exception",
    "UD: Invalid Opcode Exception",
    "NM: Device Not Avaliable Exception",
    "DF: Double Fault Exception",
    "CSO: Coprocessor Segment Overrun",
    "TS: Invalid TSS Exception",
    "NP: Segment Not Present",
    "SS: Stack Fault Exception",
    "GP: General Protection Exception",
    "PF: Page-Fault Exception",
    "RESERVED: RESERVED",
    "MF: X87 FPU Floating-Point Error",
    "AC: Alignment Check Exception",
    "MC: Machine-Check Exception",
    "XF: SIMD Floating-Point Exception"
};


/**
 * @brief This function will print out the type of the exception 
 * as well as its corrosponding index.
 * 
 * @param idx : The index we need to use to map the exception.
 */
static void exp_to_usr(int idx) {
    printf("----------------| EXCEPTION OCCURED |---------------- \n");
    printf("[Exception num: %d]", idx);
    printf(" %s \n", exception_arr[idx]);
}


/* Exception handlers */

void divide_error_handler() {
    cli();
    exp_to_usr(DIVIDE_ERROR);
    sti();
	asm volatile("iret");
}

void debug_handler() {
    exp_to_usr(DEBUG);
}

void nmi_handler() {
    exp_to_usr(NMI);
}

void int3_handler() {
    exp_to_usr(BREAKPOINT);
}

void overflow_handler() {
    exp_to_usr(OVERFLOW);
}

void bounds_handler() {
    exp_to_usr(BOUNDS_CHECK);
}

void invalid_op_handler() {
    exp_to_usr(INVALID_OPCODE);
}

void device_not_available_handler() {
    exp_to_usr(DEVICE_NOT_AVAILIAVLE);
}

void double_fault_handler() {
    exp_to_usr(DOUBLE_FAULT);
}

void coprocessor_segment_overrun_handler() {
    exp_to_usr(COPROCESSOR_OVERRUN);
}

void invalid_TSS_handler() {
    exp_to_usr(INVALID_TSS);
}

void segment_not_present_handler() {
    exp_to_usr(SEGMENT_NOT_PRESENT);
}

void stack_segment_handler() {
    exp_to_usr(STACK_SEGMENT_FAULT);
}

void general_protection_handler() {
    exp_to_usr(GENRAL_PROTECTION);
}

void page_fault_handler() {
    exp_to_usr(PAGE_FAULT);
}

void coprocessor_error_handler() {
    exp_to_usr(FLOATING_POINT_ERROR);
}

void alignment_check_handler() {
    exp_to_usr(ALIGHMENT_CHECK);
}

void machine_check_handler() {
    exp_to_usr(MACHINE_CHECK);
}

void simd_coprocessor_error_handler() {
    exp_to_usr(SIMD_FLOATING_POINT);
}
