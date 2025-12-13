#pragma once

#include <utility>
#include <cstring>

#include <libreborn/log.h>
#include <trampoline/guest.h>

#include "../common/common.h"

// Compile Trampoline Arguments
template <typename T>
__attribute__((hot, always_inline)) static inline void _handle_trampoline_arg(unsigned char *&out, const T &arg) {
    block_pointer(T);
    align_up_to_boundary(out, alignof(T));
    constexpr size_t size = sizeof(T);
    memcpy(out, &arg, size);
    out += size;
}

// Array Specialization
#include "array.h"

// Main Trampoline Function
template <typename... Args>
__attribute__((hot, always_inline)) static inline unsigned int _trampoline(const unsigned int id, const bool allow_early_return, const Args&... args) {
    // Create Arguments
    static unsigned char out[MAX_TRAMPOLINE_ARGS_SIZE];
    unsigned char *end = out;
    (_handle_trampoline_arg(end, args), ...); // https://stackoverflow.com/a/25683817
    const uint32_t length = end - out;
    // Call
    uint32_t ret = 0;
    const uint32_t err = raw_trampoline(id, allow_early_return ? nullptr : &ret, length, out);
    if (err != 0) [[unlikely]] {
        ERR("Trampoline Error: %u", err);
    }
    return ret;
}
#define trampoline(...) _trampoline(_id, ##__VA_ARGS__)

// Macro
#define CALL(unique_id, name, return_type, args) \
    return_type name args { \
        static unsigned char _id = unique_id;

// Handle Cached GL State When Switching To Offscreen Context
MCPI_INTERNAL void _media_backup_gl_state();
MCPI_INTERNAL void _media_restore_gl_state();
