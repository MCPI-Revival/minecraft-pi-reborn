#include <libreborn/libreborn.h>

#include "host.h"

// Registration
static handler_t *handlers[256];
void _add_handler(unsigned char id, handler_t *handler) {
    if (handlers[id]) {
        ERR("Conflicting Trampolines For ID: %i", (int) id);
    }
    handlers[id] = handler;
}

// Trampoline
void trampoline(g2h_t g2h, uint32_t id, uint32_t *args) {
    handlers[id](g2h, args);
}