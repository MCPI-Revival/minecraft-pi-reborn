#pragma once

#include <cstdlib>
#include <cstring>

#include <trampoline/host.h>

#include "../common/common.h"

// Macro
typedef uint32_t handler_t(trampoline_writer_t writer, const unsigned char *args);
MCPI_INTERNAL void _add_handler(unsigned char id, handler_t *handler);
#define CALL(unique_id, name, ignored1, ignored2) \
    static handler_t _run_##name; \
    __attribute__((constructor)) static void _init_##name() { \
        _add_handler(unique_id, _run_##name); \
    } \
    static uint32_t _run_##name(MCPI_UNUSED trampoline_writer_t writer, const unsigned char *raw_args) { \
        MCPI_UNUSED TrampolineArguments args(raw_args); \
        static constexpr typeof(name) *func = name;

// Arguments
struct TrampolineArguments {
    // Constructor
    explicit TrampolineArguments(const unsigned char *args) {
        raw_args = args;
        position = 0;
    }

    // Read Next Value
    template <typename T>
    const T &next() {
        block_pointer(T);
        const T &ret = *(const T *) raw_args;
        raw_args += sizeof(T);
        return ret;
    }
    // Read Next Array
    template <typename T>
    const T *next_arr(uint32_t *length = nullptr) {
        block_pointer(T);
        const uint32_t size = next<uint32_t>();
        if (length != nullptr) {
            *length = size / sizeof(T);
        }
        static bool just_read_pointer = should_just_send_pointer();
        if (size == 0) {
            return nullptr;
        } else if (just_read_pointer) {
            return (const T *) (uintptr_t) (QEMU_GUEST_BASE + next<uint32_t>());
        } else {
            const T *ret = (const T *) raw_args;
            raw_args += size;
            return ret;
        }
    }

private:
    const unsigned char *raw_args;
    unsigned int position;
};
