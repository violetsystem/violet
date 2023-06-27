#ifndef _LOG_H
#define _LOG_H 1

#include <stdarg.h>

void log_print(const char *str);
void log_printv(const char *fmt, va_list args);
void log_printf(const char *fmt, ...);

static inline void log_error(const char *fmt, ...) {
    log_print("[kernel] err: ");
    va_list args;
    va_start(args, fmt);
    log_printv(fmt, args);
    va_end(args);
}

static inline void log_warning(const char *fmt, ...) {
    log_print("[kernel] wrn: ");
    va_list args;
    va_start(args, fmt);
    log_printv(fmt, args);
    va_end(args);
}

static inline void log_info(const char *fmt, ...) {
    log_print("[kernel] inf: ");
    va_list args;
    va_start(args, fmt);
    log_printv(fmt, args);
    va_end(args);
}

#endif // _LOG_H
