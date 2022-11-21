#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#define TIMER_INTR      0x20 
#define KEYBOARD_INTR   0x21
#define RTC_INTR        0x28


void keyboard_handler(void);
void rtc_handler(void);
void timer_handler(void);


#endif /*_INTERRUPT_H_ */
