#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Patching Functions

#ifdef REBORN_HAS_PATCH_CODE

void reborn_init_patch();

// Replace Call Located At start With A Call To target
void _overwrite_call(const char *file, int line, void *start, void *target);
#define overwrite_call(start, target) _overwrite_call(__FILE__, __LINE__, start, target)

#define _check_if_method_is_new(name) \
    { \
        if (!__is_new_method_##name()) { \
            ERR("Method Is Not \"New\""); \
        } \
    }

#define _setup_fancy_overwrite(start, name, target) \
    _check_if_method_is_new(name); \
    static name##_t _original_for_##target = start; \
    static name##_t _helper_for_##target = __overwrite_helper_for_##name(target, _original_for_##target)

// Replace All Calls To Method start With target
void _overwrite_calls(const char *file, int line, void *start, void *target);
#define overwrite_calls_manual(start, target) _overwrite_calls(__FILE__, __LINE__, start, target)
#define overwrite_calls(start, target) \
    { \
        _setup_fancy_overwrite(start, start, target); \
        overwrite_calls_manual((void *) start, (void *) _helper_for_##target); \
        start = _helper_for_##target; \
    }

// Replace All Calls To Virtual Method start With target
#define overwrite_virtual_calls(start, target) \
    { \
        _setup_fancy_overwrite(*start##_vtable_addr, start, target); \
        overwrite_calls_manual((void *) *start##_vtable_addr, (void *) _helper_for_##target); \
    }

// Replace All Calls To start With target Within [to, from)
void _overwrite_calls_within(const char *file, int line, void *from, void *to, void *start, void *target);
#define overwrite_calls_within(from, to, start, target) _overwrite_calls_within(__FILE__, __LINE__, from, to, start, target)

// Get Target Address From BL Instruction
void *extract_from_bl_instruction(unsigned char *from);

// Replace Method start With target
void _overwrite(const char *file, int line, void *start, void *target);
#define overwrite_manual(start, target) _overwrite(__FILE__, __LINE__, (void *) start, (void *) target)
#define overwrite(start, target) \
    { \
        _check_if_method_is_new(start); \
        start##_t type_check = target; \
        overwrite_manual(start, (void *) type_check); \
    }

// Patch Instruction
void _patch(const char *file, int line, void *start, unsigned char patch[4]);
#define patch(start, patch) _patch(__FILE__, __LINE__, start, patch)

// Patch 4 Bytes Of Data
void _patch_address(const char *file, int line, void *start, void *target);
#define patch_address(start, target) _patch_address(__FILE__, __LINE__, (void *) start, (void *) target)

// Patch VTable Entry
// This does not affect sub-classes.
#define patch_vtable(start, target) \
    { \
        start##_t type_check = target; \
        patch_address(start##_vtable_addr, (void *) type_check); \
    }

#endif

#ifdef __cplusplus
}
#endif
