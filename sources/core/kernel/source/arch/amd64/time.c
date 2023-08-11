#include <impl/time.h>
#include <arch/include.h>
#include ARCH_INCLUDE(hpet.h)

void sleep(ms_t ms){
    hpet_sleep(ms);
}

ms_t get_current_time(void){
    return hpet_get_current_time();
}