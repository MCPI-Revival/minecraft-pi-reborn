#pragma once

#include <cstring>
#include <cstdlib>
#include <utility>

#include <libreborn/log.h>

#include "../common/common.h"

// Compile Trampoline Arguments
template <typename T>
void _handle_trampoline_arg(unsigned char *&out, const T &arg) {
    block_pointer(T);
    *(T *) out = arg;
    out += sizeof(T);
}
// Array Specialization
struct copy_array {
    template <typename T>
    copy_array(uint32_t length, T *arr) {
        block_pointer(T);
        if (arr == nullptr) {
            length = 0;
        }
        size = length * sizeof(T);
        data = arr;
    }
    explicit copy_array(const char *str) {
        size = str != nullptr ? (strlen(str) + 1) : 0;
        data = str;
    }
    uint32_t size;
    const void *data;
};
template <>
inline void _handle_trampoline_arg<copy_array>(unsigned char *&out, const copy_array &arg) {
    // Send Size
    _handle_trampoline_arg(out, arg.size);
    // Send Data
    if (arg.size > 0) {
        static bool just_send_pointer = !is_trampoline_pipe_based();
        if (just_send_pointer) {
            _handle_trampoline_arg(out, uint32_t(arg.data));
        } else {
            memcpy(out, arg.data, arg.size);
            out += arg.size;
        }
    }
}
// Variadic Templates
__attribute__((unused)) static void _add_to_trampoline_args(__attribute__((unused)) unsigned char *&out) {
}
template <typename T, typename... Args>
void _add_to_trampoline_args(unsigned char *&out, const T &first, Args&&... args) {
    _handle_trampoline_arg(out, first);
    _add_to_trampoline_args(out, std::forward<Args>(args)...);
}

// Main Trampoline Function
template <typename... Args>
unsigned int _trampoline(const unsigned int id, const bool allow_early_return, Args&&... args) {
    // Create Arguments
    static unsigned char out[MAX_TRAMPOLINE_ARGS_SIZE];
    unsigned char *end = out;
    _add_to_trampoline_args(end, std::forward<Args>(args)...);
    const uint32_t length = end - out;
    // Call
    uint32_t ret = 0;
    const uint32_t err = raw_trampoline(id, allow_early_return ? nullptr : &ret, length, out);
    if (err != 0) {
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
__attribute__((visibility("internal"))) void _media_backup_gl_state();
__attribute__((visibility("internal"))) void _media_restore_gl_state();
