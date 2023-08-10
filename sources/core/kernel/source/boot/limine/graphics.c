#include <boot/limine.h>

#include <global/term.h>

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

void graphics_init(void) {
    if(framebuffer_request.response->framebuffer_count) {
        struct limine_framebuffer* framebuffer = framebuffer_request.response->framebuffers[0];
        init_term(framebuffer->address, framebuffer->width, framebuffer->height, framebuffer->pitch, 0, 0);
    }
}