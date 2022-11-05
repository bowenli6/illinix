#include <boot/syscall.h>
#include <io.h>

/**
 * @brief A system call handler.
 *
 */
asmlinkage int32_t sys_halt(uint8_t status) {
   return 0;
}

asmlinkage int32_t sys_execute(const uint8_t *cmd) {
   return 0;
}

asmlinkage int32_t sys_open(const uint8_t *filename) {
   return 0;
}

asmlinkage int32_t sys_close(int32_t fd) {
   return 0;
}

asmlinkage int32_t sys_read(int32_t fd, void *buf, uint32_t nbytes) {
   return 0;
}

asmlinkage int32_t sys_write(int32_t fd, const void *buf, uint32_t nbytes) {
   return 0;
}

asmlinkage int32_t sys_getargs(uint8_t *buf, int32_t nbytes) {
   return 0;
}

asmlinkage int32_t sys_vidmap(uint8_t **screen_start) {
   return 0;
}

asmlinkage int32_t sys_set_handler(int32_t signum, void *handler_addr) {
   return 0;
}

asmlinkage int32_t sys_sigreturn(void) {
   return 0;
}
