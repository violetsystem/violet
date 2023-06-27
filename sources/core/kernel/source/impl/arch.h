#ifndef _ARCH_H
#define _ARCH_H 1

/**
 * Everything needed to initialize PMM/VMM
*/
void arch_stage1(void);

#include <stdnoreturn.h>

noreturn void arch_idle(void);
noreturn void arch_reboot(void);
noreturn void arch_shutdown(void);

#endif // _ARCH_H
