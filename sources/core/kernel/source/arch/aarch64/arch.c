#include <impl/arch.h>

void arch_stage1(void) {

}

noreturn void arch_idle(void) {
    for (;;) {
        asm volatile("wfi");
    }
}

noreturn void arch_reboot(void) {
    // todo
    arch_idle();
}

noreturn void arch_shutdown(void) {
    // todo
    arch_idle();
}
