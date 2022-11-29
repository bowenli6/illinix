# Details

Date : 2022-11-28 20:43:14

Directory /home/bowen/Developer/ECE391/VM/ece391/ece391_share/work/dev/illinix

Total : 111 files,  6584 codes, 2578 comments, 2095 blanks, all 11257 lines

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)

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
| [student-distrib/Makefile](/student-distrib/Makefile) | Makefile | 24 | 7 | 12 | 43 |
| [student-distrib/debug.sh](/student-distrib/debug.sh) | Shell Script | 18 | 1 | 4 | 23 |
| [student-distrib/drivers/fs.c](/student-distrib/drivers/fs.c) | C | 130 | 77 | 47 | 254 |
| [student-distrib/drivers/keyboard.c](/student-distrib/drivers/keyboard.c) | C | 32 | 15 | 14 | 61 |
| [student-distrib/drivers/rtc.c](/student-distrib/drivers/rtc.c) | C | 87 | 55 | 30 | 172 |
| [student-distrib/drivers/terminal.c](/student-distrib/drivers/terminal.c) | C | 201 | 106 | 60 | 367 |
| [student-distrib/drivers/time.c](/student-distrib/drivers/time.c) | C | 25 | 28 | 14 | 67 |
| [student-distrib/drivers/vga.c](/student-distrib/drivers/vga.c) | C | 0 | 0 | 1 | 1 |
| [student-distrib/include/access.h](/student-distrib/include/access.h) | C++ | 28 | 0 | 7 | 35 |
| [student-distrib/include/boot/exception.h](/student-distrib/include/boot/exception.h) | C++ | 45 | 2 | 7 | 54 |
| [student-distrib/include/boot/i8259.h](/student-distrib/include/boot/i8259.h) | C++ | 25 | 12 | 13 | 50 |
| [student-distrib/include/boot/idt.h](/student-distrib/include/boot/idt.h) | C++ | 7 | 0 | 6 | 13 |
| [student-distrib/include/boot/interrupt.h](/student-distrib/include/boot/interrupt.h) | C++ | 9 | 0 | 6 | 15 |
| [student-distrib/include/boot/multiboot.h](/student-distrib/include/boot/multiboot.h) | C++ | 51 | 11 | 12 | 74 |
| [student-distrib/include/boot/page.h](/student-distrib/include/boot/page.h) | C++ | 53 | 0 | 22 | 75 |
| [student-distrib/include/boot/syscall.h](/student-distrib/include/boot/syscall.h) | C++ | 20 | 49 | 24 | 93 |
| [student-distrib/include/boot/x86_desc.h](/student-distrib/include/boot/x86_desc.h) | C++ | 160 | 31 | 44 | 235 |
| [student-distrib/include/debug.h](/student-distrib/include/debug.h) | C++ | 23 | 3 | 8 | 34 |
| [student-distrib/include/drivers/fs.h](/student-distrib/include/drivers/fs.h) | C++ | 50 | 7 | 20 | 77 |
| [student-distrib/include/drivers/keyboard.h](/student-distrib/include/drivers/keyboard.h) | C++ | 24 | 1 | 9 | 34 |
| [student-distrib/include/drivers/rtc.h](/student-distrib/include/drivers/rtc.h) | C++ | 20 | 16 | 13 | 49 |
| [student-distrib/include/drivers/terminal.h](/student-distrib/include/drivers/terminal.h) | C++ | 24 | 0 | 9 | 33 |
| [student-distrib/include/drivers/time.h](/student-distrib/include/drivers/time.h) | C++ | 16 | 13 | 15 | 44 |
| [student-distrib/include/drivers/vga.h](/student-distrib/include/drivers/vga.h) | C++ | 0 | 0 | 1 | 1 |
| [student-distrib/include/errno.h](/student-distrib/include/errno.h) | C++ | 44 | 2 | 5 | 51 |
| [student-distrib/include/io.h](/student-distrib/include/io.h) | C++ | 15 | 0 | 6 | 21 |
| [student-distrib/include/kmalloc.h](/student-distrib/include/kmalloc.h) | C++ | 61 | 0 | 17 | 78 |
| [student-distrib/include/lib.h](/student-distrib/include/lib.h) | C++ | 132 | 26 | 25 | 183 |
| [student-distrib/include/list.h](/student-distrib/include/list.h) | C++ | 34 | 0 | 18 | 52 |
| [student-distrib/include/pro/pid.h](/student-distrib/include/pro/pid.h) | C++ | 9 | 0 | 5 | 14 |
| [student-distrib/include/pro/process.h](/student-distrib/include/pro/process.h) | C++ | 96 | 9 | 30 | 135 |
| [student-distrib/include/pro/sched.h](/student-distrib/include/pro/sched.h) | C++ | 49 | 26 | 26 | 101 |
| [student-distrib/include/rbtree.h](/student-distrib/include/rbtree.h) | C++ | 11 | 1 | 10 | 22 |
| [student-distrib/include/tests/tests.h](/student-distrib/include/tests/tests.h) | C++ | 7 | 1 | 5 | 13 |
| [student-distrib/include/types.h](/student-distrib/include/types.h) | C++ | 18 | 8 | 13 | 39 |
| [student-distrib/include/vfs/dentry.h](/student-distrib/include/vfs/dentry.h) | C++ | 0 | 9 | 6 | 15 |
| [student-distrib/include/vfs/file.h](/student-distrib/include/vfs/file.h) | C++ | 17 | 4 | 7 | 28 |
| [student-distrib/include/vfs/vfs.h](/student-distrib/include/vfs/vfs.h) | C++ | 24 | 0 | 9 | 33 |
| [student-distrib/kernel/access.c](/student-distrib/kernel/access.c) | C | 22 | 21 | 8 | 51 |
| [student-distrib/kernel/exception.c](/student-distrib/kernel/exception.c) | C | 86 | 9 | 26 | 121 |
| [student-distrib/kernel/file.c](/student-distrib/kernel/file.c) | C | 18 | 11 | 4 | 33 |
| [student-distrib/kernel/i8259.c](/student-distrib/kernel/i8259.c) | C | 45 | 25 | 12 | 82 |
| [student-distrib/kernel/idt.c](/student-distrib/kernel/idt.c) | C | 83 | 86 | 26 | 195 |
| [student-distrib/kernel/kernel.c](/student-distrib/kernel/kernel.c) | C | 130 | 29 | 36 | 195 |
| [student-distrib/kernel/kmalloc.c](/student-distrib/kernel/kmalloc.c) | C | 349 | 139 | 67 | 555 |
| [student-distrib/kernel/pid.c](/student-distrib/kernel/pid.c) | C | 21 | 29 | 15 | 65 |
| [student-distrib/kernel/process.c](/student-distrib/kernel/process.c) | C | 263 | 208 | 123 | 594 |
| [student-distrib/kernel/rbtree.c](/student-distrib/kernel/rbtree.c) | C | 1 | 0 | 2 | 3 |
| [student-distrib/kernel/sched.c](/student-distrib/kernel/sched.c) | C | 355 | 364 | 183 | 902 |
| [student-distrib/kernel/syscall.c](/student-distrib/kernel/syscall.c) | C | 69 | 84 | 37 | 190 |
| [student-distrib/kernel/vfs.c](/student-distrib/kernel/vfs.c) | C | 169 | 137 | 80 | 386 |
| [student-distrib/kernel/vm.c](/student-distrib/kernel/vm.c) | C | 216 | 54 | 67 | 337 |
| [student-distrib/lib/io.c](/student-distrib/lib/io.c) | C | 162 | 53 | 31 | 246 |
| [student-distrib/lib/lib.c](/student-distrib/lib/lib.c) | C | 208 | 102 | 23 | 333 |
| [student-distrib/tests/tests.c](/student-distrib/tests/tests.c) | C | 370 | 166 | 73 | 609 |
| [student-distrib/usr/app/app.c](/student-distrib/usr/app/app.c) | C | 58 | 2 | 17 | 77 |
| [student-distrib/usr/app/app.h](/student-distrib/usr/app/app.h) | C++ | 1 | 0 | 1 | 2 |
| [student-distrib/usr/stdlib/alloc.c](/student-distrib/usr/stdlib/alloc.c) | C | 115 | 115 | 52 | 282 |
| [student-distrib/usr/stdlib/alloc.h](/student-distrib/usr/stdlib/alloc.h) | C++ | 41 | 33 | 23 | 97 |
| [student-distrib/usr/stdlib/stdlib.h](/student-distrib/usr/stdlib/stdlib.h) | C++ | 4 | 2 | 4 | 10 |
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

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)