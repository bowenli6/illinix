#ifndef _TIME_H_
#define _TIME_H_


#define HZ      100     /* 10 ms */
#define TIMER   0x20    /* Interrupt vector for timer interrupt */

void do_timer(void);

#endif /* _TIME_H_ */