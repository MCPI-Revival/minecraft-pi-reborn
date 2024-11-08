#pragma once

#include <cstring>
#include <utility>

// Duplicate VTable
template <typename T>
T *extend_dup_vtable(T *vtable) {
    // Get Size
    const unsigned char *const real_vtable = ((const unsigned char *) vtable) - sizeof(void *);
    constexpr size_t real_vtable_size = sizeof(T) + sizeof(void *);
    // Allocate
    unsigned char *const new_vtable = (unsigned char *) ::operator new(real_vtable_size);
    T *ret = (T *) (new_vtable + sizeof(void *));
    new (ret) T;
    // Copy
    memcpy(new_vtable, real_vtable, real_vtable_size);
    // Return
    return ret;
}

// Customize VTable
template <typename T, void (*setup_vtable)(T *)>
T *extend_get_vtable() {
    static T *vtable = nullptr;
    if (!vtable) {
        vtable = extend_dup_vtable(T::base);
        setup_vtable(vtable);
    }
    return vtable;
}
#define CUSTOM_VTABLE(name, parent) \
    static void setup_##name##_vtable(parent##_vtable *); \
    static parent##_vtable *get_##name##_vtable() { \
        return extend_get_vtable<parent##_vtable, setup_##name##_vtable>(); \
    } \
    static void setup_##name##_vtable(parent##_vtable *vtable)

// Extend MCPI Classes
template <typename Super, typename Self>
Self *extend_get_data(Super *super) {
    return (Self *) (super + 1);
}
template <typename Super, typename Self>
auto extend_struct(auto&&... args) -> decltype(Super::allocate()) {
    constexpr size_t size = sizeof(Super) + sizeof(Self);
    Super *super = (Super *) ::operator new(size);
    Self *self = extend_get_data<Super, Self>(super);
    new (self) Self(std::forward<decltype(args)>(args)...);
    return super;
}

// Helpers
#define CREATE_HELPER(name) \
    struct Custom##name { \
        explicit Custom##name(auto&&... args): super(((name *) this) - 1) { \
            super->constructor(std::forward<decltype(args)>(args)...); \
            super->vtable = get_vtable(); \
        } \
        name *const super; \
        static name##_vtable *get_vtable(); \
    private: \
        static void setup_vtable(name##_vtable *vtable); \
    protected: \
        virtual ~Custom##name() = default; \
    public:
#include "Screen.h"
#include "DynamicTexture.h"
#undef CREATE_HELPER