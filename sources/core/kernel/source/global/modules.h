#ifndef _GLOBAL_MODULES_H
#define _GLOBAL_MODULES_H 1

#include <lib/modules/vfs.h>
#include <lib/modules/pci.h>

#define MODULE_TYPE_VFS     0
#define MODULE_TYPE_PCI     1

typedef uint8_t module_type_t;

typedef struct{
    int (*init)(int argc, char* argv[]);
	int (*fini)(void);
    char* name;
}module_metadata_t;

extern vfs_t* vfs_handler;
extern pci_t* pci_handler;

void modules_init(void);
int modules_request_dependency(module_type_t type);

#endif // _GLOBAL_MODULES_H