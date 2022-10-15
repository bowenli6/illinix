#include "../include/interrupt_list.h"


static char* interrupt_arr[2] = {
    "KEYBOARD: Keybord Interrupt",
    "RTC: Real Time Clock Interrupt"
};

/*
    Func: This function will print out the type of the interrupt 
          as well as its corrosponding index 
    Input: The index we need to use to map the interrupt arr
    Output: None
*/
void int_to_usr(int idx) {
    printf("----------------| Interrupt OCCURED |---------------- \n");
    printf("%s occured \n", interrupt_arr[idx]);
}

void KEY_BOARD_HANDLER () {
    int_to_usr(0);
}

void RTC_HANDLER () {
    int_to_usr(1);
}