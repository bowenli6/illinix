============================
Real-Time Clock
============================

---------------
Description
---------------
The DS12885, DS12887, and DS12C887 real-time
clocks (RTCs) are designed to be direct replacements
for the DS1285 and DS1287. The devices provide a
real-time clock/calendar, one time-of-day alarm, three
maskable interrupts with a common interrupt output, a
programmable square wave, and 114 bytes of batterybacked static RAM (113 bytes in the DS12C887 and
DS12C887A). The DS12887 integrates a quartz crystal
and lithium energy source into a 24-pin encapsulated
DIP package. The DS12C887 adds a century byte at
address 32h. For all devices, the date at the end of the
month is automatically adjusted for months with fewer
than 31 days, including correction for leap years. The
devices also operate in either 24-hour or
12-hour format with an AM/PM indicator. A precision
temperature-compensated circuit monitors the status of
VCC. If a primary power failure is detected, the device
automatically switches to a backup supply. A lithium
coin-cell battery can be connected to the VBAT input
pin on the DS12885 to maintain time and date operation
when primary power is absent. The device is accessed
through a multiplexed byte-wide interface, which supports both Intel and Motorola modes.


--------------
Source Code
--------------
student-distrib/include/drivers/rtc.h

student-distrib/drivers/rtc.c
