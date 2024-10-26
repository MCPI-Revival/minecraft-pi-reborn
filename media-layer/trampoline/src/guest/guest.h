#pragma once

#include <cstring>

#include "../common/common.h"

// Trampoline Function
uint32_t _raw_trampoline(uint32_t id, bool allow_early_return, uint32_t length, const unsigned char *args);

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
        this->size = length * sizeof(T);
        this->data = arr;
    }
    explicit copy_array(const char *str) {
        this->size = str != nullptr ? (strlen(str) + 1) : 0;
        this->data = str;
    }
    uint32_t size;
    const void *data;
};
template <>
inline void _handle_trampoline_arg<copy_array>(unsigned char *&out, const copy_array &arg) {
    *(uint32_t *) out = arg.size;
    out += sizeof(uint32_t);
    if (arg.size > 0) {
        memcpy(out, arg.data, arg.size);
        out += arg.size;
    }
}
// Variadic Templates
__attribute__((unused)) static void _add_to_trampoline_args(__attribute__((unused)) unsigned char *&out) {
}
template <typename T, typename... Args>
void _add_to_trampoline_args(unsigned char *&out, const T &first, const Args... args) {
    _handle_trampoline_arg(out, first);
    _add_to_trampoline_args(out, args...);
}

// Main Trampoline Function
template <typename... Args>
unsigned int _trampoline(const unsigned int id, const bool allow_early_return, Args... args) {
    static unsigned char out[MAX_TRAMPOLINE_ARGS_SIZE];
    unsigned char *end = out;
    _add_to_trampoline_args(end, args...);
    const uint32_t length = end - out;
    return _raw_trampoline(id, allow_early_return, length, out);
}
#define trampoline(...) _trampoline(_id, ##__VA_ARGS__)

// Macro
#define CALL(unique_id, name, return_type, args) \
    return_type name args { \
        static unsigned char _id = unique_id;

// Handle Cached GL State When Switching To Offscreen Context
__attribute__((visibility("internal"))) void _media_backup_gl_state();
__attribute__((visibility("internal"))) void _media_restore_gl_state();
