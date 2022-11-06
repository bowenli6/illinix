#include <pro/process.h>
#include <boot/x86_desc.h>
#include <errno.h>  

/* this map will indicate which task is active now */
process_t    task_map[TASK_COUNT];
process_t    curr_process;
// tss is the current tss we are using


/**
 * @brief store the entire command line witout the leading file name
 * 
 * @return 0  if the program executes a halt system call
 * @return -1 if the command cannot be executed
 */
int32_t parse_arg_to_process(uint8_t* command, uint8_t* stored_pro, uint8_t* stored_file) {
    if (command == NULL) {
        printf("The command itself is null, we can get no information. \n");
        return -1;                          /* return -1 if the command can not be executed */
    }

    /* parse the command into file name and the rest of the command */


    return 0;
}


/**
 * @brief this function is used user mode to kernel mode
 * 
 * @return none
 */
void usr_to_kernel() {

}


/**
 * @brief this function is used for kernel mode to usr mode
 * 
 * @return none
 */
void kernel_to_usr() {
    
    uint32_t* address_for_eip;
    uint32_t EIP_reg, EFLAGS_reg, ESP_reg;
    uint16_t CS_reg, DS_reg;

    address_for_eip = (uint32_t*) (PROGRAM_IMG_BEGIN + EIP_OFFSET);

    EIP_reg = *(address_for_eip);
    CS_reg = GDT_USR_CS | USR_LEVEL;
    DS_reg = GDT_USR_DS | USR_LEVEL;
    EFLAGS_reg = tss.eflags;
    ESP_reg = USR_STACK_SZ + VIR_MEM_BEGIN;                 /* which starts from 132MB */

    /* store the ss0 and esp0 to the tss */
    tss.ss0 = curr_process.tss_SS0;
    tss.esp0 = curr_process.tss_ESP0;


    // asm volatile("              \n\
            
    //         "
    //         : "=a"(val)
    //         : "d"(port)
    //         : "memory"
    // );



}


void process_create() {
    



}