#pragma once

#define PROXY_LOG_TAG "(Media Layer Proxy Server) "

// Assign Unique ID To Function
__attribute__((visibility("internal"))) void _assign_unique_id(const char *name, unsigned char id);

// Must Call After Every Call
__attribute__((visibility("internal"))) void _start_proxy_call(const char *name);
#define start_proxy_call() _start_proxy_call(__func__)
__attribute__((visibility("internal"))) void end_proxy_call();

#define CALL(unique_id, name, return_type, args) \
    __attribute__((constructor)) static void _init_##name() { \
        _assign_unique_id(#name, unique_id); \
    } \
    return_type name args
