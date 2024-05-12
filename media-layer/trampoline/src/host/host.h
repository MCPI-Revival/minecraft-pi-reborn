#pragma once

#include <stdint.h>

// Trampoline Function
typedef void *(*g2h_t)(uint32_t guest_addr);
void trampoline(g2h_t g2h, uint32_t id, uint32_t *args); // See trampoline.patch

// Macro
typedef void handler_t(g2h_t g2h, uint32_t *args);
__attribute__((visibility("internal"))) void _add_handler(unsigned char id, handler_t *handler);
#define CALL(unique_id, name, ignored1, ignored2) \
    static handler_t _run_##name; \
    __attribute__((constructor)) static void _init_##name() { \
        _add_handler(unique_id, _run_##name); \
    } \
    static void _run_##name(__attribute__((unused)) g2h_t g2h, __attribute__((unused)) uint32_t *args) { \
        __attribute__((unused)) int _current_arg = 0; \
        static typeof(name) *func = name;

// Helper Macros
#define next_int() args[_current_arg++]
#define next_ptr() g2h(next_int())
#define next_float() pun_to(float, next_int())
#define ret(x) \
    args[0] = x; \
    return;
