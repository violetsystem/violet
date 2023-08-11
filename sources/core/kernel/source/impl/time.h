#ifndef _IMPL_TIME_H
#define _IMPL_TIME_H 1

#include <stdint.h>

typedef uint64_t ms_t; // microsecond

void sleep(ms_t ms);
ms_t get_current_time(void);

#endif // _SERIAL_H
