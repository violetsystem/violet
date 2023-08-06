#include <impl/pmm.h>
#include <boot/limine.h>

#include <lib/math.h>
#include <lib/bitmap.h>
#include <lib/memory.h>

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

uint8_t *bitmap = NULL;

uint64_t available_pages = 0;
uint64_t used_pages = 0;
uint64_t reserved_pages = 0;

uint64_t highest_page_index = 0;
uint64_t last_used_index = 0;

void pmm_init(void) {
    struct limine_memmap_entry **entries = memmap_request.response->entries;
    uint64_t entry_count = memmap_request.response->entry_count;

    uint64_t highest_address = 0;

    for (size_t i = 0; i < entry_count; i++) {
        struct limine_memmap_entry *entry = entries[i];

        switch (entry->type) {
            case LIMINE_MEMMAP_USABLE:
                available_pages += DIV_ROUNDUP(entry->length, PAGE_SIZE);
                highest_address = MAX(highest_address, entry->base + entry->length);
                break;
            case LIMINE_MEMMAP_RESERVED:
            case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
            case LIMINE_MEMMAP_ACPI_NVS:
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
            case LIMINE_MEMMAP_KERNEL_AND_MODULES:
                reserved_pages += DIV_ROUNDUP(entry->length, PAGE_SIZE);
                break;
        }
    }

    highest_page_index = highest_address / PAGE_SIZE;
    uint64_t bitmap_size = ALIGN_UP(highest_page_index / 8, PAGE_SIZE);

    for (size_t i = 0; i < entry_count; i++) {

        struct limine_memmap_entry *entry = entries[i];

        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        if (entry->length >= bitmap_size) {
            bitmap = (uint8_t*)(entry->base + hhdm_request.response->offset);

            memset(bitmap, 0xff, bitmap_size);

            entry->length -= bitmap_size;
            entry->base += bitmap_size;

            break;
        }
        
    }

    for (size_t i = 0; i < entry_count; i++) {
        struct limine_memmap_entry *entry = entries[i];

        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        for (uint64_t j = 0; j < entry->length; j += PAGE_SIZE) {
            bitmap_reset(bitmap, (entry->base + j) / PAGE_SIZE);
        }
    }

}
