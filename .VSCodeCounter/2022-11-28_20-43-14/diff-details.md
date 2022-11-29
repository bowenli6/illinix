# Diff Details

Date : 2022-11-28 20:43:14

Directory /home/bowen/Developer/ECE391/VM/ece391/ece391_share/work/dev/illinix

Total : 51 files,  2209 codes, 389 comments, 605 blanks, all 3203 lines

[Summary](results.md) / [Details](details.md) / [Diff Summary](diff.md) / Diff Details

## Files
| filename | language | code | comment | blank | total |
| :--- | :--- | ---: | ---: | ---: | ---: |
| [Documentation/Boot/GDT.rst](/Documentation/Boot/GDT.rst) | reStructuredText | 23 | 0 | 6 | 29 |
| [Documentation/Boot/IDT.rst](/Documentation/Boot/IDT.rst) | reStructuredText | 59 | 0 | 22 | 81 |
| [Documentation/Boot/PIC.rst](/Documentation/Boot/PIC.rst) | reStructuredText | 20 | 0 | 6 | 26 |
| [Documentation/Boot/Paging.rst](/Documentation/Boot/Paging.rst) | reStructuredText | 43 | 0 | 24 | 67 |
| [Documentation/Devices/Filesystem.rst](/Documentation/Devices/Filesystem.rst) | reStructuredText | 30 | 0 | 8 | 38 |
| [Documentation/Devices/Keyboard.rst](/Documentation/Devices/Keyboard.rst) | reStructuredText | 19 | 0 | 6 | 25 |
| [Documentation/Devices/RTC.rst](/Documentation/Devices/RTC.rst) | reStructuredText | 32 | 0 | 5 | 37 |
| [Documentation/Devices/Terminal.rst](/Documentation/Devices/Terminal.rst) | reStructuredText | 13 | 0 | 6 | 19 |
| [Documentation/Syscall/syscall.rst](/Documentation/Syscall/syscall.rst) | reStructuredText | 120 | 0 | 70 | 190 |
| [Documentation/VFS/virtual-file-system.rst](/Documentation/VFS/virtual-file-system.rst) | reStructuredText | 24 | 0 | 10 | 34 |
| [README.md](/README.md) | Markdown | 43 | 0 | 28 | 71 |
| [fish/Makefile](/fish/Makefile) | Makefile | 16 | 2 | 8 | 26 |
| [fish/blink.h](/fish/blink.h) | C++ | 14 | 0 | 2 | 16 |
| [fish/ece391emulate.c](/fish/ece391emulate.c) | C | 191 | 11 | 26 | 228 |
| [fish/ece391support.c](/fish/ece391support.c) | C | 44 | 0 | 10 | 54 |
| [fish/ece391support.h](/fish/ece391support.h) | C++ | 8 | 0 | 3 | 11 |
| [fish/ece391syscall.h](/fish/ece391syscall.h) | C++ | 12 | 7 | 6 | 25 |
| [fish/ece391sysnum.h](/fish/ece391sysnum.h) | C++ | 13 | 0 | 3 | 16 |
| [fish/fish.c](/fish/fish.c) | C | 156 | 1 | 40 | 197 |
| [include/errno.h](/include/errno.h) | C++ | 45 | 1 | 5 | 51 |
| [include/math.h](/include/math.h) | C++ | 0 | 0 | 1 | 1 |
| [include/stdio.h](/include/stdio.h) | C++ | 3 | 0 | 3 | 6 |
| [include/stdlib.h](/include/stdlib.h) | C++ | 3 | 0 | 3 | 6 |
| [include/string.h](/include/string.h) | C++ | 19 | 0 | 6 | 25 |
| [include/type.h](/include/type.h) | C++ | 9 | 0 | 4 | 13 |
| [include/unistd.h](/include/unistd.h) | C++ | 3 | 0 | 3 | 6 |
| [lib/errno.c](/lib/errno.c) | C | 0 | 0 | 1 | 1 |
| [lib/math.c](/lib/math.c) | C | 0 | 0 | 1 | 1 |
| [lib/stdio.c](/lib/stdio.c) | C | 0 | 0 | 1 | 1 |
| [lib/stdlib.c](/lib/stdlib.c) | C | 0 | 0 | 1 | 1 |
| [lib/string.c](/lib/string.c) | C | 186 | 152 | 49 | 387 |
| [lib/unistd.c](/lib/unistd.c) | C | 0 | 0 | 1 | 1 |
| [syscalls/Makefile](/syscalls/Makefile) | Makefile | 19 | 0 | 8 | 27 |
| [syscalls/ece391cat.c](/syscalls/ece391cat.c) | C | 25 | 0 | 8 | 33 |
| [syscalls/ece391counter.c](/syscalls/ece391counter.c) | C | 37 | 0 | 9 | 46 |
| [syscalls/ece391emulate.c](/syscalls/ece391emulate.c) | C | 191 | 11 | 26 | 228 |
| [syscalls/ece391grep.c](/syscalls/ece391grep.c) | C | 86 | 2 | 10 | 98 |
| [syscalls/ece391hello.c](/syscalls/ece391hello.c) | C | 18 | 0 | 7 | 25 |
| [syscalls/ece391ls.c](/syscalls/ece391ls.c) | C | 23 | 0 | 7 | 30 |
| [syscalls/ece391main.c](/syscalls/ece391main.c) | C | 24 | 0 | 3 | 27 |
| [syscalls/ece391pingpong.c](/syscalls/ece391pingpong.c) | C | 63 | 13 | 14 | 90 |
| [syscalls/ece391shell.c](/syscalls/ece391shell.c) | C | 31 | 0 | 6 | 37 |
| [syscalls/ece391sigtest.c](/syscalls/ece391sigtest.c) | C | 69 | 0 | 13 | 82 |
| [syscalls/ece391support.c](/syscalls/ece391support.c) | C | 76 | 10 | 19 | 105 |
| [syscalls/ece391support.h](/syscalls/ece391support.h) | C++ | 10 | 0 | 4 | 14 |
| [syscalls/ece391syscall.h](/syscalls/ece391syscall.h) | C++ | 22 | 7 | 7 | 36 |
| [syscalls/ece391syserr.c](/syscalls/ece391syserr.c) | C | 237 | 71 | 39 | 347 |
| [syscalls/ece391sysnum.h](/syscalls/ece391sysnum.h) | C++ | 13 | 0 | 3 | 16 |
| [syscalls/ece391testprint.c](/syscalls/ece391testprint.c) | C | 8 | 0 | 6 | 14 |
| [usr/bsh.c](/usr/bsh.c) | C | 103 | 87 | 45 | 235 |
| [usr/ps.c](/usr/ps.c) | C | 6 | 14 | 3 | 23 |

[Summary](results.md) / [Details](details.md) / [Diff Summary](diff.md) / Diff Details