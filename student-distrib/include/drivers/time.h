#ifndef _TIME_H_
#define _TIME_H_

#define HZ                  1000            /* 100 timer interrupts per second (10 ms) */
#define TICKUNIT            1000000UL       /* 1 ms = 1000,000 nanoseconds */
#define CLOCK_TICK_RATE     1193182         /* 8254 chip's internal oscillator frequency */
#define LATCH   ((CLOCK_TICK_RATE + HZ / 2) / HZ)
#define CMD_REG 0x43
#define TIMER_CHANNEL   0x40
#define TIMER_IRQ       0


/* timer object */
typedef struct {
    /* records the exact time of last tick; it is invoked by the timer interrupt handler */
    // mark_offset; 

    /* returns the time elapsed since last tick */
    // get_offset;

    /* returns the number of nanoseconds since kernel boot */
    // monotonic_clock;

    /* waits for a given number of "loops" */
    // delay;
} timer_t;


typedef struct {
    /* stores the number of seconds that have elsaped since midnight of January 1 1970 (UTC) */
    // tv_sec;

    /* stores the number of nanoseconds that have elapsed within the last second */
    // tv_nsec;
} timespec;


void pit_init(void);
void do_timer(void);


#endif /* _TIME_H_ */
