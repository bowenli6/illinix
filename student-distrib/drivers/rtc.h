
/*Generic RTC interface*/

#ifndef RTC_H_
#define RTC_H_


#define RTC_CMD_port  0x70
#define RTC_DATA_port 0x71

#define RTC_A_reg 0x8A
#define RTC_B_reg 0x8B
#define RTC_C_reg 0x8C

#define IRQ_8 8

/**
 * RTC_init
 * description: initialize RTC and enable RTC interrupt
 * input: void
 * output: void
 */

void RTC_init(void);

/**
 * RTC_interrupt
 * description: Read data from register C and handle it
 * input: void
 * output: void
 */

void RTC_handler(void);






#endif RTC_H_



