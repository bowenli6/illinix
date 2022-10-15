#include "../include/IDT.h"

#define SYSCALL_IDX 0x80

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

void SYSCAL_HANDLER () {
    printf("----------------| SYSCALL OCCURED |---------------- \n");
}

void idt_init() {
    int i;
    for (i = 0; i < IDT_SIZE; i ++) {
        /* Set up each IDT descriptor value */
        idt[i].seg_selector = KERNEL_CS;
        idt[i].present = 0;
        idt[i].reserved0 = 0;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved3 = 1;
        idt[i].reserved4 = 0;

        /* Update the prividge level */
        idt[i].dpl = 0;
    }

    SET_IDT_ENTRY(idt[0], DIVIDE_ZERO_HANDLER);
    SET_IDT_ENTRY(idt[1], DEBUG_HANDLER);
    SET_IDT_ENTRY(idt[2], NON_MASKABLE_HANDLER);
    SET_IDT_ENTRY(idt[3], BREAKPOINT_HANDLER);
    SET_IDT_ENTRY(idt[4], OVERFLOW_HANDLER);
    SET_IDT_ENTRY(idt[5], RANGE_EXCEEDED_HANDLER);
    SET_IDT_ENTRY(idt[6], INVALID_OP_HANDLER);
    SET_IDT_ENTRY(idt[7], DEVICE_UNALIBLE_HANDLER);
    SET_IDT_ENTRY(idt[8], DOUBLE_FAULT_HANDLER);
    SET_IDT_ENTRY(idt[9], COPROCESSOR_OVERRUN_HANDLER);
    SET_IDT_ENTRY(idt[10], INVALID_TSS_HANDLER);
    SET_IDT_ENTRY(idt[11], SEGMENT_NOT_PRESENT_HANDLER);
    SET_IDT_ENTRY(idt[12], STACK_FAULT_HANDLER);
    SET_IDT_ENTRY(idt[13], GENRAL_PROTECTION_HANDLER);
    SET_IDT_ENTRY(idt[14], PAGE_FAULT_HANDLER);
    SET_IDT_ENTRY(idt[15], RESERVED_HANDLER);
    SET_IDT_ENTRY(idt[16], X87_FLOATING_POINT_HANDLER);
    SET_IDT_ENTRY(idt[17], ALIGHMENT_CHECK_HANDLER);
    SET_IDT_ENTRY(idt[18], MACHINE_CHECK_HANDLER);
    SET_IDT_ENTRY(idt[19], SIMD_FLOATING_POINT_HANDLER);

    SET_IDT_ENTRY(idt[KEY_BOARD_IDX], KEY_BOARD_HANDLER);
    idt[KEY_BOARD_IDX].reserved3 = 0;

    SET_IDT_ENTRY(idt[RTC_IDX], RTC_HANDLER);
    idt[RTC_IDX].reserved3 = 0;

    SET_IDT_ENTRY(idt[SYSCALL_IDX], SYSCAL_HANDLER);
    idt[SYSCALL_IDX].dpl = 3;
}

