#include <global/modules.h>

module_metadata_t module_metadata = {
    "test"
};

void print_terminal(const char* str);

void main(void){
    print_terminal("Hello world from module");
}