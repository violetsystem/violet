#include <arch/include.h>
#include ARCH_INCLUDE(cpu.h)
#include ARCH_INCLUDE(asm.h)

void simd_init(void) {
    asm_write_cr0(asm_read_cr0() & ~((uint64_t)CR0_EMULATION)); 
    asm_write_cr0(asm_read_cr0() | CR0_MONITOR_CO_PROCESSOR);
    asm_write_cr0(asm_read_cr0() | CR0_NUMERIC_ERROR_ENABLE);

    asm_write_cr4(asm_read_cr4() | CR4_FXSR_ENABLE);
    asm_write_cr4(asm_read_cr4() | CR4_SIMD_EXCEPTION_SUPPORT);
    
    asm_fninit();
}

void simd_save(void* location) {
    asm volatile("fxsave (%0) "::"r"(location));
}

void simd_restore(void* location) {
    asm volatile("fxrstor (%0) "::"r"(location));
}