#ifndef _MODULES_STORAGE_H
#define _MODULES_STORAGE_H 1

#include <stdint.h>
#include <stddef.h>

typedef struct storage_device_t{
    int (*read)(struct storage_device_t*, uint64_t, size_t, void*);
    int (*write)(struct storage_device_t*, uint64_t, size_t, void*);
    uint64_t storage_size;
    void* internal_data;
} storage_device_t;

#endif // _MODULES_VFS_H