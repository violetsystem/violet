#ifndef _AMD64_IO_H
#define _AMD64_IO_H 1

#include <stdint.h>

static inline void io_write8(uint16_t port, uint8_t data){
    __asm__ volatile("outb %b0, %w1" : : "a" (data), "Nd" (port));
}

static inline void io_write16(uint16_t port, uint16_t data){
    __asm__ volatile("outw %w0, %w1" : : "a" (data), "Nd" (port));
}

static inline void io_write32(uint16_t port, uint32_t data){
	__asm__ volatile("outl %0, %w1" : : "a" (data), "Nd" (port));
}

static inline uint8_t io_read8(uint16_t port){
    uint8_t data = 0;
    __asm__ volatile("inb %w1, %b0" : "=a" (data) : "Nd" (port));
    return data;
}

static inline uint16_t io_read16(uint16_t port){
    uint16_t data = 0;
    __asm__ volatile("inw %w1, %w0" : "=a" (data) : "Nd" (port));
    return data;
}

static inline uint32_t io_read32(uint16_t port){
    uint32_t data = 0;
    __asm__ volatile("inl %w1, %0" : "=a" (data) : "Nd" (port));
    return data;
}

#endif // _AMD64_IO_H
