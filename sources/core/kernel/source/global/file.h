#ifndef _GLOBAL_FILE_H
#define _GLOBAL_FILE_H 1

#include <stdint.h>
#include <stddef.h>
#include <lib/modules/file.h>

file_t* open(const char* path, int flags);

#endif // _GLOBAL_FILE_H