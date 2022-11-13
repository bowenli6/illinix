#include <boot/syscall.h>
#include <pro/process.h>
#include <io.h>


asmlinkage int32_t sys_restart(void) {
   return -1;
}

asmlinkage int32_t sys_set_handler(int32_t signum, void *handler_addr) {
   return -1;
}

asmlinkage int32_t sys_sigreturn(void) {
   return -1;
}
