#ifndef _PID_H_
#define _PID_H_

#include <pro/process.h>

#define PID_SIZE    0x8000    /* Total bits of a 4KB page */
#define BITMAP_SIZE 0x1000    /* 0x8000 / 8 */

pid_t alloc_pid();
pid_t kill_pid();
void pidmap_init();

#endif
