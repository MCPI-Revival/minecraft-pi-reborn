#pragma once

#define PROXY_LOG_TAG "(Media Layer Proxy Client) "

typedef void (*proxy_handler_t)();
__attribute__((visibility("internal"))) void _add_handler(unsigned char id, proxy_handler_t handler);

#define CALL(unique_id, name, return_type, args) \
    static void _run_##name (); \
    __attribute__((constructor)) static void _init_##name() { \
        _add_handler(unique_id, _run_##name); \
    } \
    static void _run_##name ()
