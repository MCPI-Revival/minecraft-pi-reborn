#include <libreborn/libreborn.h>

#include "host.h"

// Registration
static handler_t *handlers[256];
void _add_handler(const unsigned char id, handler_t *handler) {
    if (handlers[id]) {
        ERR("Conflicting Trampolines For ID: %i", (int) id);
    }
    handlers[id] = handler;
}

// Trampoline
uint32_t trampoline(const trampoline_writer_t writer, const uint32_t id, const unsigned char *args) {
    return handlers[id](writer, args);
}