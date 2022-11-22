#ifndef _TIME_H_
#define _TIME_H_


#define HZ                  100         /* 10 ms */
#define CLOCK_TICK_RATE     1193180     /* clock tick rate*/
#define LATCH   ((CLOCK_TICK_RATE + HZ / 2) / HZ)
#define CMD_REG 0x43
#define TIMER_CHANNEL   0x40
#define TIMER_IRQ       0



void PIT_init(void);
void do_timer(void);

#endif /* _TIME_H_ */
