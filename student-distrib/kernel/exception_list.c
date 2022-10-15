#include "../include/exception_list.h"

/* According to IA32 page 6, we define the name for first 20 exceptions*/
static char* exception_arr[EXCEPTION_COUNT] = {
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

/*
    Func: This function will print out the type of the exception 
          as well as its corrosponding index 
    Input: The index we need to use to map the exception arr
    Output: None
*/
void exp_to_usr(int idx) {
    printf("----------------| EXCEPTION OCCURED |---------------- \n");
    printf("[Exception num: %d]", idx);
    printf(" %s \n", exception_arr[idx]);
}

void DIVIDE_ZERO_HANDLER  () {
    (exp_to_usr(0));
}

void DEBUG_HANDLER  () {
    (exp_to_usr(1));
}

void NON_MASKABLE_HANDLER  () {
    (exp_to_usr(2));
}

void BREAKPOINT_HANDLER  () {
    (exp_to_usr(3));
}

void OVERFLOW_HANDLER  () {
    (exp_to_usr(4));
}

void RANGE_EXCEEDED_HANDLER  () {
    (exp_to_usr(5));
}

void INVALID_OP_HANDLER  () {
    (exp_to_usr(6));
}

void DEVICE_UNALIBLE_HANDLER  () {
    (exp_to_usr(7));
}

void DOUBLE_FAULT_HANDLER  () {
    (exp_to_usr(8));
}

void COPROCESSOR_OVERRUN_HANDLER  () {
    (exp_to_usr(9));
}

void INVALID_TSS_HANDLER  () {
    (exp_to_usr(10));
}

void SEGMENT_NOT_PRESENT_HANDLER  () {
    (exp_to_usr(11));
}

void STACK_FAULT_HANDLER  () {
    (exp_to_usr(12));
}

void GENRAL_PROTECTION_HANDLER  () {
    (exp_to_usr(13));
}

void PAGE_FAULT_HANDLER  () {
    (exp_to_usr(14));
}

void RESERVED_HANDLER  () {
    (exp_to_usr(15));
}

void X87_FLOATING_POINT_HANDLER  () {
    (exp_to_usr(16));
}

void ALIGHMENT_CHECK_HANDLER  () {
    (exp_to_usr(17));
}

void MACHINE_CHECK_HANDLER  () {
    (exp_to_usr(18));
}

void SIMD_FLOATING_POINT_HANDLER  () {
    (exp_to_usr(19));
}
