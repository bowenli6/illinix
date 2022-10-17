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

void do_divide_error() {
    // exp_to_usr(DIVIDE_ERROR);
    printf("d");
}

void do_debug() {
    exp_to_usr(DEBUG);
}

void do_nmi() {
    exp_to_usr(NMI);
}

void do_int3() {
    exp_to_usr(BREAKPOINT);
}

void do_overflow() {
    exp_to_usr(OVERFLOW);
}

void do_bounds() {
    exp_to_usr(BOUNDS_CHECK);
}

void do_invalid_op() {
    exp_to_usr(INVALID_OPCODE);
}

void do_device_not_available() {
    exp_to_usr(DEVICE_NOT_AVAILIAVLE);
}

void do_double_fault() {
    exp_to_usr(DOUBLE_FAULT);
}

void do_coprocessor_segment_overrun() {
    exp_to_usr(COPROCESSOR_OVERRUN);
}

void do_invalid_TSS() {
    exp_to_usr(INVALID_TSS);
}

void do_segment_not_present() {
    exp_to_usr(SEGMENT_NOT_PRESENT);
}

void do_stack_segment() {
    exp_to_usr(STACK_SEGMENT_FAULT);
}

void do_general_protection() {
    exp_to_usr(GENRAL_PROTECTION);
}

void do_page_fault() {
    exp_to_usr(PAGE_FAULT);
}

void do_coprocessor_error() {
    exp_to_usr(FLOATING_POINT_ERROR);
}

void do_alignment_check() {
    exp_to_usr(ALIGHMENT_CHECK);
}

void do_machine_check() {
    exp_to_usr(MACHINE_CHECK);
}

void do_simd_coprocessor_error() {
    exp_to_usr(SIMD_FLOATING_POINT);
}
