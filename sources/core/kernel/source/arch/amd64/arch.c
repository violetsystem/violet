#include <impl/arch.h>

#include <arch/amd64/lib/gdt.h>

void arch_stage1(void) {
    asm volatile("cli");
    gdt_init();
}

noreturn void arch_idle(void) {
    for (;;) {
        asm volatile("hlt");
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
