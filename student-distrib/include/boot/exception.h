#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#define EXCEPTION_COUNT             20

/* Exceptions */
typedef enum {
    DIVIDE_ERROR,
    DEBUG,
    NMI,
    BREAKPOINT,
    OVERFLOW,
    BOUNDS_CHECK,
    INVALID_OPCODE,
    DEVICE_NOT_AVAILIAVLE,
    DOUBLE_FAULT,
    COPROCESSOR_OVERRUN,
    INVALID_TSS,
    SEGMENT_NOT_PRESENT,
    STACK_SEGMENT_FAULT,
    GENRAL_PROTECTION,
    PAGE_FAULT,
    INTEL_RESERVED,
    FLOATING_POINT_ERROR,
    ALIGHMENT_CHECK,
    MACHINE_CHECK,
    SIMD_FLOATING_POINT
} exception_t;

/* Exceptions handlers */

void divide_error_handler();
void debug_handler();
void nmi_handler();
void int3_handler();
void overflow_handler();
void bounds_handler();
void invalid_op_handler();
void device_not_available_handler();
void double_fault_handler();
void coprocessor_segment_overrun_handler();
void invalid_TSS_handler();
void segment_not_present_handler();
void stack_segment_handler();
void general_protection_handler();
void page_fault_handler();
void coprocessor_error_handler();
void alignment_check_handler();
void machine_check_handler();
void simd_coprocessor_error_handler();


#endif /* _EXCEPTION_H */
