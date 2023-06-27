#include <impl/serial.h>
#include <impl/boot.h>
#include <impl/arch.h>
#include <impl/pmm.h>
#include <impl/vmm.h>

#include <lib/log.h>

#include <kernel.h>

/**
 * _start have to be called with 64 bits enabled!
 * it is preferable to make the less things before jumping into _start
*/
void _start(void) {
    serial_init();
    log_info("version  = %s %lu.%lu\n", KERNEL_VERSION, KERNEL_MAJOR, KERNEL_MINOR);
    log_info("branch   = %s\n", KERNEL_BRANCH);
    log_info("arch     = %s\n", KERNEL_ARCH);
    log_info("protocol = %s\n", BOOT_PROTOCOL);

    arch_stage1();
    pmm_init();
    vmm_init();
    log_print("\n");
    log_info("memory available = %lu MiB\n", pmm_get_available() / 0x100000);
    log_info("memory reserved  = %lu MiB\n", pmm_get_reserved() / 0x100000);

    arch_idle();
}
