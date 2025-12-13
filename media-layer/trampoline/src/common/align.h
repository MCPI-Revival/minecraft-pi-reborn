#pragma once

// Verify Alignment Values Are Consistent On Guest/Host
#define check(type, value) static_assert(alignof(type) == (value))
check(bool, 1);
check(char, 1);
check(float, 4);
check(int, 4);
check(double, 8);
#undef check

// Align Pointer To Boundary
#define def(...) \
    template <typename T> \
    __attribute__((hot, always_inline)) static inline void align_up_to_boundary(__VA_ARGS__ &ptr) { \
        constexpr int align = alignof(T); \
        const uintptr_t diff = uintptr_t(ptr) % align; \
        if (diff > 0) { \
            ptr += align - diff; \
        } \
    }
def(unsigned char *)
def(const unsigned char *)
def(uintptr_t)
#undef def