#ifndef _PANIC_H
#define _PANIC_H 1

#include <stdnoreturn.h>

noreturn void panic(const char *fmt, ...);

#endif // _PANIC_H
