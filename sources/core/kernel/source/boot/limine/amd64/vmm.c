#include <impl/vmm.h>

#include <arch/include.h>
#include ARCH_INCLUDE(vmm.h)

void vmm_init(void) {
    /* limine map all we need, so all we have to do is preload the empty fields (the higher half part of the table: VMM_HALF_TABLE - VMM_END_TABLE) into the last pagination level. */
    kernel_space = vmm_get_current_space();

    vmm_clear_lower_half_entries(kernel_space);
    vmm_preload_higher_half_entries(kernel_space);
}
