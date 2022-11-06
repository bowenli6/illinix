#include <boot/syscall.h>
#include <pro/process.h>
#include <io.h>

asmlinkage int32_t sys_halt(uint8_t status) {
   return 0;
}

asmlinkage int32_t sys_execute(const uint8_t *cmd) {
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
