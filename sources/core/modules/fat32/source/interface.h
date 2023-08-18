#ifndef PARTITION_INTERFACE_H
#define PARTITION_INTERFACE_H

#include <fat_types.h>

typedef struct{
    void* volume;
    uint64_t start;
    uint64_t size;
    struct storage_device_t* device;
} partition_t;

u8 partition_read(partition_t* partition, u8* buffer, u32 lba, u32 count);

u8 partition_write(partition_t* partition, const u8* buffer, u32 lba, u32 count);

#endif // PARTITION_INTERFACE_H