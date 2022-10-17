#ifndef _RTC_H
#define _RTC_H


#define RTC_CMD_port  0x70
#define RTC_DATA_port 0x71

#define RTC_A_reg 0x8A
#define RTC_B_reg 0x8B
#define RTC_C_reg 0x8C

#define RTC_IRQ 8

void RTC_init();
void do_RTC();

#endif /* _RTC_H */
