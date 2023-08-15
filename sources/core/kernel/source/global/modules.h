#ifndef _GLOBAL_MODULES_H
#define _GLOBAL_MODULES_H 1

#include <lib/modules/vfs.h>

typedef struct{
    int (*init)(int argc, char* argv[]);
	int (*fini)(void);
    char* name;
}module_metadata_t;

extern vfs_t* vfs_handler;

#endif // _GLOBAL_MODULES_H