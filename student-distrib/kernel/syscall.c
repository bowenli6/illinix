#include <boot/syscall.h>
#include <pro/process.h>
#include <io.h>


asmlinkage int32_t sys_restart(void) {
   return 0;
}

asmlinkage int32_t sys_set_handler(int32_t signum, void *handler_addr) {
   return 0;
}

asmlinkage int32_t sys_sigreturn(void) {
   return 0;
}
