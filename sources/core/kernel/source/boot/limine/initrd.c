#include <boot/limine.h>

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <lib/string.h>
#include <lib/memory.h>
#include <global/modules.h>

static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0
};

static struct limine_file* initrd_get_file_ptr(const char* path){
    if(module_request.response){
        for(uint64_t i = 0; i < module_request.response->module_count; i++){
            if(!strcmp(module_request.response->modules[i]->path, path)){
                return module_request.response->modules[i];
            }
        }
    }

    return NULL;
}

static vfs_t early_vfs_handler;

file_t* initrd_get_file(const char* path, int flags){
    return (file_t*)initrd_get_file_ptr(path);
}

file_t* initrd_get_file_base(file_t* file_ptr){
    if(file_ptr){
        struct limine_file* file = (struct limine_file*)file_ptr;
        return file->address;
    }

    return NULL;
}

int initrd_read_file(file_t* file_ptr, void* base, size_t size){
    if(file_ptr){
        struct limine_file* file = (struct limine_file*)file_ptr;
        size_t size_to_copy = size;
        if(size_to_copy > file->size){
            size_to_copy = file->size;
        }
        memcpy(base, file->address, size_to_copy);
        return 0;
    }

    return -EINVAL;
}

ssize_t initrd_get_file_size(file_t* file_ptr){
    if(file_ptr){
        struct limine_file* file = (struct limine_file*)file_ptr;
        return file->size;
    }

    return -EINVAL;
}

void initrd_init(void) {
    early_vfs_handler.open = &initrd_get_file;
    vfs_handler = &early_vfs_handler;
}