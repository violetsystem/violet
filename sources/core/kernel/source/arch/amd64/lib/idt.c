#include <arch/amd64/lib/idt.h>
#include <arch/amd64/lib/gdt.h>

// extern uintptr_t __interrupt_vector[256];

static __attribute__((aligned(4096))) idt_t _idt = {};

static idtr_t _idtr = {
    .limit = sizeof(idt_t) - 1,
    .base = (uint64_t) &_idt,
};

idt_entry_t idt_entry(uintptr_t handler, uint8_t ist, uint8_t idt_flags) {
    return (idt_entry_t) {
        .offset_low = (handler),
        .code_segment = GDT_KERNEL_CODE * 8,
        .ist = ist,
        .attributes = idt_flags,
        .offset_middle = (handler >> 16),
        .offset_high = (handler >> 32),
        .zero = 0,
    };
}

void idt_init(void) {
    // for (uint8_t i = 0; i < 256; i++) {
    //     _idt.entries[i] = idt_entry(__interrupt_vector[i], 0, IDT_GATE);
    // }
    // idt_update(&_idtr);
}
