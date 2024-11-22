#pragma once

#include "log.h"

// Patching Functions
#ifndef REBORN_HAS_PATCH_CODE
#error "Missing Patching Functions"
#endif

// Init
void reborn_init_patch();

// Replace Call Located At start With A Call To target
void overwrite_call(void *start, void *target, bool force_b_instruction = false);

// Replace All Calls To Method start With target
void *overwrite_calls_manual(void *start, void *target, bool allow_no_callsites = false);
template <typename T>
void overwrite_calls(T *target, typename T::overwrite_type replacement) {
    DEBUG("Overwriting Method: %s", target->name);
    if (!target->overwrite(replacement)) {
        ERR("Unable To Overwrite Method!");
    }
}

// Thunk Enabler
void *reborn_thunk_enabler(void *target, void *thunk);

// Replace All Calls To start With target Within [to, from)
void overwrite_calls_within_manual(void *from, void *to, void *start, void *target);
template <typename T>
void overwrite_calls_within(void *from, void *to, T *start, typename T::ptr_type target) {
    overwrite_calls_within_manual(from, to, (void *) start->get(), (void *) target);
}

// Get Target Address From BL Instruction
void *extract_from_bl_instruction(unsigned char *from);

// Patch Instruction
void patch(void *start, unsigned char patch[4]);

// Patch 4 Bytes Of Data
void patch_address(void *start, void *target);

// Patch VTable Entry
// This does not affect subclasses.
template <typename T>
void patch_vtable(const T *start, typename T::ptr_type target) {
    DEBUG("Patching VTable: %s", start->name);
    if (start->enabled) {
        WARN("Use overwrite_calls() Instead!");
    }
    patch_address((void *) start->get_vtable_addr(), (void *) target);
}