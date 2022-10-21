#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#define KEYBOARD_INTR 0x21
#define RTC_INTR      0x28


void keyboard_handler();
void rtc_handler();


#endif /*_INTERRUPT_H */
