#pragma once

#include <mods/extend/extend.h>

// VTable Patching
#define PATCH_VTABLE(name) \
    vtable->name = [](_Self *self, auto... args) { \
        return extend_get_data<_Self, _Data>(self)->name(std::forward<decltype(args)>(args)...); \
    }
#define _PATCH_VTABLE_DESTRUCTOR(name, type) \
    vtable->type = [](_Self *self) { \
        extend_get_data<_Self, _Data>(self)->~_Data(); \
        return name##_vtable::base->type(self); \
    }
#define _PATCH_VTABLE_DESTRUCTORS(name) \
    _PATCH_VTABLE_DESTRUCTOR(name, destructor_complete); \
    _PATCH_VTABLE_DESTRUCTOR(name, destructor_deleting)
#define SETUP_VTABLE(name) \
    name##_vtable *Custom##name::get_vtable() { \
        return extend_get_vtable<name##_vtable, setup_vtable>(); \
    } \
    typedef name _Self; \
    typedef Custom##name _Data; \
    void Custom##name::setup_vtable(name##_vtable *vtable) { \
        _PATCH_VTABLE_DESTRUCTORS(name);
