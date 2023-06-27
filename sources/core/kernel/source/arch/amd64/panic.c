#include <impl/panic.h>
#include <impl/arch.h>
#include <lib/log.h>

noreturn void panic(const char *fmt, ...) {
    log_print("[kernel] pnc: ");
    va_list args;
    va_start(args, fmt);
    log_printv(fmt, args);
    va_end(args);
    arch_idle();
}
