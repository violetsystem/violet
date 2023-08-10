#include <impl/arch.h>

#include <arch/include.h>
#include ARCH_INCLUDE(idt.h)
#include ARCH_INCLUDE(gdt.h)
#include ARCH_INCLUDE(simd.h)

void arch_stage1(void) {
    __asm__ volatile("cli");
    gdt_init();
    //idt_init();
}

void arch_stage2(void) {
    simd_init();
}

noreturn void arch_idle(void) {
    for (;;) {
        __asm__ volatile("hlt");
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
