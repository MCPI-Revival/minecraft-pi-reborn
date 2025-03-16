#pragma once

#include <mods/extend/extend.h>

// VTable Patching
#define PATCH_VTABLE(name) \
    vtable->name = [](_Data::_Self *self, auto... args) { \
        return extend_get_data<_Data>(self)->name(std::forward<decltype(args)>(args)...); \
    }
#define _PATCH_VTABLE_DESTRUCTOR(type) \
    vtable->type = [](_Data::_Self *self) { \
        extend_get_data<_Data>(self)->~_Data(); \
        return _Data::_VTable::base->type(self); \
    }
#define _PATCH_VTABLE_DESTRUCTORS(name) \
    _PATCH_VTABLE_DESTRUCTOR(destructor_complete); \
    _PATCH_VTABLE_DESTRUCTOR(destructor_deleting)
#define SETUP_VTABLE(name) \
    typedef Custom##name _Data; \
    _Data::_VTable *Custom##name::get_vtable() { \
        return extend_get_vtable<_Data::_VTable, setup_vtable>(); \
    } \
    void _Data::setup_vtable(_Data::_VTable *vtable) { \
        _PATCH_VTABLE_DESTRUCTORS();
