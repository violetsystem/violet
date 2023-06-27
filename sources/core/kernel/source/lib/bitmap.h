#ifndef _BITMAP_H
#define _BITMAP_H 1

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

static inline bool bitmap_get(uint8_t *bitmap, size_t bit) {
    return bitmap[bit / 8] & (1 << (bit % 8));
}

static inline void bitmap_set(uint8_t *bitmap, size_t bit) {
    bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void bitmap_reset(uint8_t *bitmap, size_t bit) {
    bitmap[bit / 8] &= ~(1 << (bit % 8));
}

#endif // _BITMAP_H
