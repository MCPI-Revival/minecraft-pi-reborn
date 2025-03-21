#pragma once

#include <cstring>
#include <utility>

#include <symbols/minecraft.h>

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
template <typename Data>
Data *extend_get_data(typename Data::_Self *self) {
    return (Data *) (self + 1);
}
template <typename Data, typename Self = typename Data::_Self>
auto extend_struct(auto&&... args) -> decltype(Self::allocate()) {
    constexpr size_t size = sizeof(Self) + sizeof(Data);
    Self *out = (Self *) ::operator new(size);
    Data *data = extend_get_data<Data>(out);
    new (data) Data(std::forward<decltype(args)>(args)...);
    return out;
}

// Helpers
#define CREATE_HELPER(name) \
    struct Custom##name { \
        typedef name _Self; \
        typedef name##_vtable _VTable; \
        _Self *const self; \
        __PREVENT_COPY(Custom##name); \
        template <typename... Args> \
        explicit Custom##name(Args&&... args): self(((_Self *) this) - 1) { \
            self->constructor(std::forward<Args>(args)...); \
            self->vtable = get_vtable(); \
        } \
        static _VTable *get_vtable(); \
    private: \
        static void setup_vtable(_VTable *vtable); \
    protected: \
        virtual ~Custom##name() = default; \
    public:
#include "internal/Screen.h"
#include "internal/DynamicTexture.h"
#include "internal/Item.h"
#include "internal/Tile.h"
#undef CREATE_HELPER