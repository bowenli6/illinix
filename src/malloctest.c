/**
 * @file ps.c
 * @author your name (you@domain.com)
 * @brief ps displays information about a selection of the active processes. 
 * ps selects all processes with the same effective user ID (euid=EUID) 
 * as the current user and associated with the same terminal as the invoker.  
 * It displays the process ID (pid=PID), the terminal associated with the 
 * process (tname=TTY).
 * @version 0.1
 * @date 2022-11-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define BUFSIZE 1024

int recursion_test(int t, int i)
{   
    int x[100];
    if(t == i ){
        ece391_fdputs (1, (uint8_t*)"Recursion test done. Your current memory map:\n");
        return 0;
    }
    else {
        return recursion_test(t, i+1);
    }
}

int main ()
{
    int32_t cnt, rval;
    uint8_t buf[BUFSIZE];

    while (1) {
        ece391_fdputs (1, (uint8_t*)"Malloc test. Choose test type: [0]heap allocate [1]stack allocate [2]memory map [q]quit\n");
        if (-1 == (cnt = ece391_read (0, buf, BUFSIZE-1))) {
            ece391_fdputs (1, (uint8_t*)"read from keyboard failed\n");
            return 3;
        }

        if (cnt > 0 && '\n' == buf[cnt - 1])
            cnt--;
        buf[cnt] = '\0';
        if (0 == ece391_strcmp (buf, (uint8_t*)"q"))
            return 0;
        
        if ('\0' == buf[0])
            continue;
        
        if (0 == ece391_strcmp (buf, (uint8_t*)"0"))
        {
            return 0;
        }
        if (0 == ece391_strcmp (buf, (uint8_t*)"1"))
        {
            recursion_test(100, 0);
            return 0;
        }
        if (0 == ece391_strcmp (buf, (uint8_t*)"1"))
        {  
            return 0;
        }
    }
}

