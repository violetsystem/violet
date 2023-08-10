#include <arch/amd64/lib/idt.h>
#include <arch/amd64/lib/gdt.h>

extern void* __interrupt_vector[256];

static __attribute__((aligned(4096))) idt_t _idt = {};

static idtr_t _idtr = {
    .limit = sizeof(idt_t) - 1,
    .base = (uint64_t) &_idt,
};

idt_entry_t idt_entry(void* handler, uint8_t ist, uint8_t idt_flags) {
    return (idt_entry_t) {
        .offset_low = ((uint64_t)handler) & 0xffff,
        .code_segment = GDT_KERNEL_CODE * 8,
        .ist = ist,
        .attributes = idt_flags,
        .offset_middle = ((uint64_t)handler >> 16) & 0xffff,
        .offset_high = ((uint64_t)handler >> 32) & 0xffffffff,
        .zero = 0,
    };
}

void idt_init(void) {
    for (uint16_t i = 0; i < 256; i++) {
        _idt.entries[i] = idt_entry(__interrupt_vector[i], 0, IDT_GATE);
    }
    idt_update(&_idtr);
}
