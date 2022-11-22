#ifndef _SCHED_H_
#define _SCHED_H_


#include <types.h>


void sched_init(void);
int32_t schedule(void);
void pause(void);


#endif /* _SCHED_H_ */
