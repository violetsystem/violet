#ifndef _PMM_H
#define _PMM_H 1

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 0x1000

extern uint8_t *bitmap;

extern uint64_t available_pages;
extern uint64_t used_pages;
extern uint64_t reserved_pages;

extern uint64_t highest_page_index;
extern uint64_t last_used_index;

void pmm_init(void);

static inline uint64_t pmm_get_available(void) {
    return available_pages * PAGE_SIZE;
}

static inline uint64_t pmm_get_reserved(void) {
    return reserved_pages * PAGE_SIZE;
}

static inline uint64_t pmm_get_used(void) {
    return used_pages * PAGE_SIZE;
}

#endif // _PMM_H
