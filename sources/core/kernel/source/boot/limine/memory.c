#include <boot/limine.h>

#include <impl/vmm.h>
#include <global/pmm.h>

static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

void memory_init(void) {
    hhdm_address = (void*)hhdm_request.response->offset;
    pmm_init();
    vmm_init();
}