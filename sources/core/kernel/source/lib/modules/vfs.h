#ifndef _MODULES_VFS_H
#define _MODULES_VFS_H 1

#include <lib/modules/file.h>

typedef struct{
    file_t* (*open)(const char*, int);
} vfs_t;

#endif // _MODULES_VFS_H