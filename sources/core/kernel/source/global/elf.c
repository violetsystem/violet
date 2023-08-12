#include <impl/initrd.h>
#include <global/file.h>
#include <global/modules.h>

int load_elf_module(int argc, char* args[]){
    file_t* file = open(args[0], 0);
    return 0;
}