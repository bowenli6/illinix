# Details

Date : 2022-11-28 20:41:41

Directory /home/bowen/Developer/ECE391/VM/ece391/ece391_share/work/dev/illinix/student-distrib

Total : 60 files,  4375 codes, 2189 comments, 1490 blanks, all 8054 lines

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)

## Files
| filename | language | code | comment | blank | total |
| :--- | :--- | ---: | ---: | ---: | ---: |
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

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)