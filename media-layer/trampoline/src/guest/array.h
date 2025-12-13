#pragma once

// Structure For Array Data
struct copy_array {
    template <typename T>
    copy_array(uint32_t length, T *arr) {
        block_pointer(T);
        if (arr == nullptr) {
            length = 0;
        }
        size = length * sizeof(T);
        data = arr;
        align = alignof(T);
    }
    explicit copy_array(const char *str) {
        size = str != nullptr ? (strlen(str) + 1) : 0;
        data = str;
        align = 1;
    }
    uint32_t size;
    const void *data;
    int align;
};

// Actually Copy The Array
template <>
__attribute__((hot, always_inline)) inline void _handle_trampoline_arg<copy_array>(unsigned char *&out, const copy_array &arg) {
    // Send Size
    _handle_trampoline_arg(out, arg.size);
    // Send Data
    if (arg.size > 0) {
        static bool just_send_pointer = should_just_send_pointer();
        if (just_send_pointer) {
            _handle_trampoline_arg(out, uint32_t(arg.data));
        } else {
            align_up_to_boundary(out, arg.align);
            memcpy(out, arg.data, arg.size);
            out += arg.size;
        }
    }
}
