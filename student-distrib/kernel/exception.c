#include <boot/exception.h>
#include <access.h>
#include <pro/process.h>
#include <boot/x86_desc.h>
#include <boot/page.h>
#include <io.h>
#include <kmalloc.h>


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
    exp_to_usr(DIVIDE_ERROR);
    //printf("d");
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

/**
 * @brief If the address of the page fault is within the allowed
 *        user stack range, expand the user stack by 4KB for the user.
 * 
 * @param errcode   
 * @param addr      Address of page fault.
 */
void 
do_page_fault(int errcode, int addr) 
{   

    if(addr < USER_STACK_ADDR && addr > (USER_STACK_ADDR - USER_STACK_MAX)) {
        thread_t* t;
        vm_area_t* area;
        uint32_t pa, length, va;
        uint32_t* temp;

        GETPRO(t);
        
        // printf("handling page fault.. getting more stack!\n Your process = %d, ", t->pid);
        // printf("your address: %x\n", addr);

        area = t->vm.map_list;
        while(area != 0) {
            if(area->vmflag & VM_STACK) {
                //TODO
                //vmdealloc(area, (area->vmend-area->vmstart), 1);
                length = (area->vmend - area->vmstart) / PAGE_SIZE + 1;

                area->vmstart = area->vmstart - PAGE_SIZE;
                pa = get_user_page(0);                      /* Alloc physical memory. */

                temp = kmalloc(sizeof(uint32_t*) * length);
                if(length > 1) {
                    memcpy((char*)(temp + sizeof(uint32_t*)), (char*)area->mmap, sizeof(uint32_t*) * length);
                    kfree(area->mmap);
                }
                
                area->mmap = temp;

                va = area->vmstart;
                mmap(va, pa, PAGE_SIZE, PTE_RW | PTE_US);   /* Create mmap. */

                area->mmap[0] = ADDR_TO_PTE(pa) | PTE_PRESENT | PTE_RW | PTE_US;
                // printf("------------------------------------------------------------\n");
                // printf("PAGE FAULT HANDLER: Succeed! Your new stack start: 0x%x.\n", area->vmstart);
                // printf("------------------------------------------------------------\n");
                // int i;
                // for(i = 0; i < length; i++) {
                //     printf("mmap: %x", area->mmap[i]);
                // }
                
                return;
            }
            area = area->next;
        }
        printf("ERROR: NO STACK?\n");
        while(1);
    }

    printf("PAGE FAULT! ERROR ADDRESS: %x\n", addr);
    while(1);
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
