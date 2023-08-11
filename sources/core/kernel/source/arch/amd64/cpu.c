#include <stdint.h>
#include <global/heap.h>

#include <arch/include.h>
#include ARCH_INCLUDE(cpu.h)

struct cpu_context{
    uint64_t id;
    uint64_t syscall_stack;
    uint64_t user_stack;
}__attribute__((packed));


void cpu_init(void){
    struct cpu_context* context = (struct cpu_context*)calloc(1, sizeof(struct cpu_context));
    context->id = cpu_get_apicid();
    reload_gs_fs();
    set_cpu_gs_base((uint64_t)context);
}