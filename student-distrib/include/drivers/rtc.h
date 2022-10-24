#ifndef _RTC_H
#define _RTC_H


#define RTC_CMD_port  0x70
#define RTC_DATA_port 0x71

#define RTC_A_reg 0x8A
#define RTC_B_reg 0x8B
#define RTC_C_reg 0x8C


#define RTC_MAX_freq 1024
#define RTC_MIN_freq 2
#define prev_mask 0xF0
#define rate_mask 0x0F

#define RTC_IRQ 8

#include <types.h>

/*
 * RTC_init()
 * @brief initalize the RTC and set the frequency to 1024
 * Input: None
 * Output: None
*/
void RTC_init();

/*
 * do_RTC()
 * Function: RTC handler, Read data from register C and handle it
 * Input: None
 * Output: None
*/
void do_RTC();

/*
 * RTC_open(const uint8_t* filename)
 * Function: open the RTC, return 
 * Input: const uint8_t* filename
 * Output: 0 on success, -1 otherwise
*/
int32_t RTC_open(const uint8_t* filename);

/*
 * RTC_close(const uint8_t* filename)
 * Function: close the RTC, return 0 on success
 * Input: int32_t fd -- file descriptor
 * Output: 0 on success, -1 otherwise
*/
int32_t RTC_close(int32_t fd);

/*
 * RTC_read(int32_t fd, const void* buffer, int32_t nbytes)
 * Function: wait for interrupt on RTC
 * Input: int32_t fd -- file descriptor
 *        const void* buffer -- address of the target frequency
 *        int32_t nbytes -- number of bytes, should be 4
 * Output: 0 on success, -1 otherwise
*/
int32_t RTC_read(int32_t fd, const void* buffer, int32_t nbytes);

/*
 * RTC_write(int32_t fd, const void* buffer, int32_t nbytes)
 * Function: set the RTC frequency based on buffer
 * Input: int32_t fd -- file descriptor
 *        const void* buffer -- address of the target frequency
 *        int32_t nbytes -- number of bytes, should be 4
 * Output: 0 on success, -1 otherwise
*/
int32_t RTC_write(int32_t fd, const void* buffer, int32_t nbytes);



#endif /* _RTC_H */
