#pragma once

#include "../common/common.h"

// Trampoline Function
extern "C" std::remove_pointer_t<trampoline_t> trampoline;

// Macro
typedef uint32_t handler_t(trampoline_writer_t writer, const unsigned char *args);
__attribute__((visibility("internal"))) void _add_handler(unsigned char id, handler_t *handler);
#define CALL(unique_id, name, ignored1, ignored2) \
    static handler_t _run_##name; \
    __attribute__((constructor)) static void _init_##name() { \
        _add_handler(unique_id, _run_##name); \
    } \
    static uint32_t _run_##name(__attribute__((unused)) trampoline_writer_t writer, const unsigned char *raw_args) { \
        __attribute__((unused)) TrampolineArguments args(raw_args); \
        static typeof(name) *func = name;

// Arguments
struct TrampolineArguments {
    explicit TrampolineArguments(const unsigned char *args) {
        this->raw_args = args;
        this->position = 0;
    }

    template <typename T>
    T next() {
        block_pointer(T);
        T ret = *(const T *) raw_args;
        raw_args += sizeof(T);
        return ret;
    }
    template <typename T>
    const T *next_arr(uint32_t *length = nullptr) {
        block_pointer(T);
        const uint32_t size = next<uint32_t>();
        if (length != nullptr) {
            *length = size / sizeof(T);
        }
        if (size == 0) {
            return nullptr;
        }
        const T *ret = (const T *) raw_args;
        raw_args += size;
        return ret;
    }

private:
    const unsigned char *raw_args;
    unsigned int position;
};
