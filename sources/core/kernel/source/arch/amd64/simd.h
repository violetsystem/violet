#ifndef _AMD64_SIMD_H
#define _AMD64_SIMD_H 1

void simd_init(void);

void simd_save(void* location);
void simd_restore(void* location);

#endif