#pragma once

#include "log.h"

// Patching Functions
#ifndef REBORN_HAS_PATCH_CODE
#error "Missing Patching Functions"
#endif

// Init
MCPI_REBORN_PATCH_PUBLIC void reborn_init_patch();

// Replace Call Located At start With A Call To target
MCPI_REBORN_PATCH_PUBLIC void overwrite_call_manual(void *addr, void *new_target, bool force_b_instruction = false);
template <typename T>
MCPI_REBORN_PATCH_PUBLIC void overwrite_call(void *addr, MCPI_UNUSED T *target_type, typename T::ptr_type new_target, const bool force_b_instruction = false) {
    overwrite_call_manual(addr, (void *) new_target, force_b_instruction);
}

// Replace All Calls To Method start With target
MCPI_REBORN_PATCH_PUBLIC void *overwrite_calls_manual(void *target, void *replacement, bool allow_no_callsites = false);
template <typename T>
MCPI_REBORN_PATCH_PUBLIC void overwrite_calls(T *target, typename T::overwrite_type replacement) {
    DEBUG("Overwriting Method: %s", target->name);
    if (!target->overwrite(replacement)) {
        ERR("Selected Method Does Not Actually Exist! Use patch_vtable() Instead!");
    }
}

// Thunk Enabler
MCPI_REBORN_PATCH_PUBLIC void *reborn_thunk_enabler(void *target, void *thunk);

// Replace All Calls To start With target Within [to, from)
MCPI_REBORN_PATCH_PUBLIC void overwrite_calls_within_manual(void *from, void *to, void *target, void *replacement);
template <typename T>
MCPI_REBORN_PATCH_PUBLIC void overwrite_calls_within(void *from, void *to, T *target, typename T::ptr_type replacement) {
    overwrite_calls_within_manual(from, to, (void *) target->get(false), (void *) replacement);
}

// Get Target Address From BL Instruction
MCPI_REBORN_PATCH_PUBLIC void *extract_from_bl_instruction(unsigned char *addr);

// Patch Instruction
MCPI_REBORN_PATCH_PUBLIC extern bool ignore_patch_conflict;
MCPI_REBORN_PATCH_PUBLIC void patch(void *addr, unsigned char patch[4]);
// Patch 4 Bytes Of Data
MCPI_REBORN_PATCH_PUBLIC void patch_address(void *addr, void *target);

// Patch VTable Entry
// IMPORTANT NOTE: This does not affect subclasses.
template <typename T>
MCPI_REBORN_PATCH_PUBLIC void patch_vtable(const T *target, typename T::ptr_type replacement) {
    DEBUG("Patching VTable: %s", target->name);
    if (target->enabled) {
        WARN("Use overwrite_calls() Instead!");
    }
    patch_address((void *) target->get_vtable_addr(), (void *) replacement);
}