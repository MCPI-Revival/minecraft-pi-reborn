#pragma once

// Structure For Array Data
template <typename T>
struct copy_array {
    copy_array(uint32_t length, const T *arr) {
        block_pointer(T);
        if (arr == nullptr) {
            length = 0;
        }
        size = length * sizeof(T);
        data = arr;
    }
    template <typename U = T, typename = std::enable_if_t<std::is_same_v<U, char>>>
    explicit copy_array(const T *str) {
        size = str != nullptr ? (strlen(str) + 1) : 0;
        data = str;
    }
    uint32_t size;
    const void *data;
};

// Actually Copy The Array
template <typename T>
__attribute__((hot, always_inline)) static inline void _handle_trampoline_arg(unsigned char *&out, const copy_array<T> &arg) {
    // Send Size
    _handle_trampoline_arg(out, arg.size);
    // Send Data
    if (arg.size > 0) {
        static bool just_send_pointer = should_just_send_pointer();
        if (just_send_pointer) {
            _handle_trampoline_arg(out, uint32_t(arg.data));
        } else {
            align_up_to_boundary<T>(out);
            memcpy(out, arg.data, arg.size);
            out += arg.size;
        }
    }
}
