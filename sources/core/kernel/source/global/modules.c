#include <stddef.h>
#include <lib/log.h>
#include <lib/string.h>
#include <impl/panic.h>
#include <global/heap.h>
#include <global/file.h>
#include <global/modules.h>
#include <global/elf_loader.h>

vfs_t* vfs_handler = NULL;
pci_t* pci_handler = NULL;

static const char* modules_cfg_path = "/modules.cfg";

void modules_init(void){
    file_t* file = vfs_handler->open(modules_cfg_path, 0);
    if(file != NULL){
        void* buffer = malloc(file->size);
        file->read(buffer, file->size, file);
        char* line = (char*)buffer;
        while(line != NULL){
            char* data = strchr(line, '=') + sizeof(char);
            char* end_of_line = strchr(line, '\n');
            char* current_line = line;
            if(end_of_line){
                *end_of_line = '\0';
                line = end_of_line + sizeof(char);
            }else{
                line = NULL;
            }

            if(strstr(current_line, "MODULE_PATH=")){
                char* args[2] = {data, NULL};
                load_elf_module(1, args);
            }

        }
        free(buffer);
    }else{
        panic("%s not found !", modules_cfg_path);
    }
}

int modules_request_dependency(module_type_t type){
    // TODO
    return 0;
}