#include <boot/syscall.h>
#include <io.h>

/**
 * @brief A system call handler.
 * 
 */
void do_syscall() {
    printf("----------------| SYSCALL OCCURED |---------------- \n");
}

int32_t sys_halt(uint8_t status) {
    return 0;
}
 int32_t sys_exe(const uint8_t *cmd) {
    return 0;

 }
 int32_t sys_open(const uint8_t *filename) {
    return 0;

 }
 int32_t sys_close(int32_t fd) {
        return 0;


 }
 int32_t sys_read(int32_t fd, void *buf, uint32_t nbytes) {
    return 0;

 }
 int32_t sys_write(int32_t fd, const void *buf, uint32_t nbytes) {
    return 0;

 }
