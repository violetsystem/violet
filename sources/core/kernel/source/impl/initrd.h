#ifndef _IMPL_INITRD_H
#define _IMPL_INITRD_H 1

#include <stddef.h>
#include <sys/types.h>
#include <global/modules.h>

void initrd_init(void);

file_t* initrd_get_file(const char* path, int flags);
file_t* initrd_get_file_base(file_t* file_ptr);
void initrd_read_file(file_t* file_ptr, void* base, size_t size);
ssize_t initrd_get_file_size(file_t* file_ptr);

#endif