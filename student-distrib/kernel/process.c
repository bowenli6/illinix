#include <pro/process.h>


/* this map will indicate which task is active now */
uint8_t     task_map[TASK_COUNT] = {0, 0};

// tss is the current tss we are using
    













/**
 * @brief store the entire command line witout the leading file name
 * 
 * @return 0  if the program executes a halt system call
 * @return -1 if the command cannot be executed
 */
void parse_arg_to_process(uint8_t* command, uint8_t* stored_pro, uint8_t* stored_file) {
    if (command == NULL) {
        printf("The command itself is null, we can get no information. \n");
        return -EINVAL;                          /* return -1 if the command can not be executed */
    }

    /* parse the command into file name and the rest of the command */


    return 0;
}


/**
 * @brief this function is used for context switch
 * 
 * @return none
 */
void process_context_switch(process_t* curr_pro) {

}

void process_create() {

}