#include <lib/log.h>
#include <global/modules.h>

int init(int argc, char* args[]){
    log_printf("Hello world from module");
    return 0;
}

int fini(void){
    return 0;
}

module_metadata_t module_metadata = {
    &init,
    &fini,
    "test"
};
