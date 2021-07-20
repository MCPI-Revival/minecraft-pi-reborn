#pragma once

#define PROXY_LOG_TAG "(Media Layer Proxy Server) "

// Assign Unique ID To Function
__attribute__((visibility("internal"))) void _assign_unique_id(const char *name, unsigned char id);
__attribute__((visibility("internal"))) unsigned char _get_unique_id(const char *name);

// Must Call After Every Call
__attribute__((visibility("internal"))) void _start_proxy_call(unsigned char call_id);
#define start_proxy_call() \
    { \
        static int _loaded_id = 0; \
        static unsigned char _call_id; \
        if (!_loaded_id) { \
            _loaded_id = 1; \
            _call_id = _get_unique_id(__func__); \
        } \
        _start_proxy_call(_call_id); \
    }
__attribute__((visibility("internal"))) void end_proxy_call();

#define CALL(unique_id, name, return_type, args) \
    __attribute__((constructor)) static void _init_##name() { \
        _assign_unique_id(#name, unique_id); \
    } \
    return_type name args
