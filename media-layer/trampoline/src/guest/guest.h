#pragma once

#include <stdint.h>

// Trampoline Function
uint32_t _trampoline(uint32_t id, uint32_t *args);
#define trampoline(...) _trampoline(_id, (uint32_t[]){__VA_ARGS__})

// Macro
#define CALL(unique_id, name, return_type, args) \
    return_type name args { \
        static unsigned char _id = unique_id;
