#include <impl/vmm.h>
#include <impl/boot.h>
#include <impl/arch.h>
#include <global/pmm.h>
#include <impl/serial.h>
#include <impl/memory.h>
#include <global/heap.h>
#include <impl/graphics.h>
#include <global/scheduler.h>

#include <lib/log.h>

#include <kernel.h>

void test_userspace(void);

/**
 * kernel_entry have to be called with 64 bits enabled!
 * it is preferable to make the less things before jumping into kernel_entry
*/
void kernel_entry(void) {
    serial_init();
    log_info("version  = %s %lu.%lu\n", KERNEL_VERSION, KERNEL_MAJOR, KERNEL_MINOR);
    log_info("branch   = %s\n", KERNEL_BRANCH);
    log_info("arch     = %s\n", KERNEL_ARCH);
    log_info("protocol = %s\n", BOOT_PROTOCOL);
    log_print("\n");

    arch_stage1();
    memory_init();
    log_info("memory available = %lu MiB\n", pmm_get_available() / 0x100000);
    log_info("memory reserved  = %lu MiB\n", pmm_get_reserved() / 0x100000);
    log_print("\n");

    /* graphics_init needs memory to be init*/
    graphics_init();

    arch_stage2();


    process_t* process = scheduler_create_process(PROCESS_SET_FLAG_TYPE(PROCESS_TYPE_APP));
    void* userspace_page = pmm_allocate_page();
    vmm_map(process->vmm_space, (memory_range_t){(void*)0x1000, PAGE_SIZE}, (memory_range_t){userspace_page, PAGE_SIZE}, MEMORY_FLAG_READABLE | MEMORY_FLAG_WRITABLE | MEMORY_FLAG_EXECUTABLE | MEMORY_FLAG_USER);
    memcpy(vmm_get_virtual_address(userspace_page), &test_userspace, PAGE_SIZE);
    scheduler_launcher_thread(scheduler_create_thread(process, (void*)0x1000, NULL), NULL);

    arch_idle();
}
