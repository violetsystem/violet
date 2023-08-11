#include <stdbool.h>
#include <lib/log.h>
#include <impl/arch.h>
#include <impl/panic.h>
#include <arch/include.h>
#include ARCH_INCLUDE(asm.h)
#include ARCH_INCLUDE(apic.h)
#include ARCH_INCLUDE(impl/arch.h)

char* exceptions_list[32] = {
    "DivisionByZero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
};

struct stack_frame{
    struct stack_frame* rbp;
    uint64_t rip;
}__attribute__((packed));

static bool is_panicking;

static void dump_registers(arch_context_t* ctx) {
    log_print("REGISTERS : \n");
    log_printf("rip: 0x%p | rsp: 0x%p\n", ctx->rip, ctx->rsp);
    log_printf("cr2: 0x%p | cr3: 0x%p\n", asm_read_cr2(), ctx->cr3);
    log_printf("cs : 0%p  | ss : 0%p | rflags: 0%p\n", ctx->cs, ctx->ss, ctx->rflags);

    log_printf("\n");

    log_printf("rax: 0x%p | rbx: 0x%p\n", ctx->rax, ctx->rbx);
    log_printf("rcx: 0x%p | rdx: 0x%p\n", ctx->rcx, ctx->rdx);
    log_printf("rsi: 0x%p | rdi: 0x%p\n", ctx->rsi, ctx->rdi);
    log_printf("rbp: 0x%p | r8 : 0x%p\n", ctx->rbp, ctx->r8);
    log_printf("r9 : 0x%p | r10: 0x%p\n", ctx->r9, ctx->r10);
    log_printf("r11: 0x%p | r12: 0x%p\n", ctx->r11, ctx->r12);
    log_printf("r13: 0x%p | r14: 0x%p\n", ctx->r13, ctx->r14);
    log_printf("r15: 0x%p\n", ctx->r15);
    log_print("\n");
    log_print("------------------------------------------------------------\n");
}

static void dump_backtrace(arch_context_t* ctx) {
    log_print("BACKTRACE : \n");
    struct stack_frame* frame = (struct stack_frame*)ctx->rbp;

    while (frame) {
        log_printf("- %p\n", frame->rip);
        frame = frame->rbp;
    }
    log_print("\n");
    log_print("------------------------------------------------------------\n");
}

static void interrupt_error_handler(arch_context_t* ctx, uint8_t cpu_id) {
    if(is_panicking) {
        arch_idle();
    }

    is_panicking = true;

    log_print("------------------------------------------------------------\n");
    log_print("KERNEL FATAL EXCEPTION : \n");
    log_print("\n");
    log_print("------------------------------------------------------------\n");

    dump_registers(ctx);

    dump_backtrace(ctx);

    log_print("\n");

    panic("exception : %s | error code : %d | cpu id : %d\n", exceptions_list[ctx->interrupt_number], ctx->error_code, cpu_id);
}

void interrupt_handler(arch_context_t* ctx, uint8_t cpu_id) {
    if(ctx->interrupt_number < 32) {
        interrupt_error_handler(ctx, cpu_id);
    }else{
        log_printf("Interrupt : %d CPU : %d\n", ctx->interrupt_number, cpu_id);
    }
    local_apic_eoi(cpu_id);
}