#ifndef _IMPL_ARCH_H
#define _IMPL_ARCH_H 1

/**
 * arch specific stuff that don't need pmm and vmm to be initialized or that are needed to initialize vmm
*/
void arch_stage1(void);

/**
 * arch specific stuff that need heap to be initialize
*/
void arch_stage2(void);

#include <stdnoreturn.h>

noreturn void arch_idle(void);
noreturn void arch_reboot(void);
noreturn void arch_shutdown(void);

#endif // _ARCH_H
