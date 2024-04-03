#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Patching Functions

#ifdef REBORN_HAS_PATCH_CODE

void reborn_init_patch();

void _overwrite_call(const char *file, int line, void *start, void *target);
#define overwrite_call(start, target) _overwrite_call(__FILE__, __LINE__, start, target)

#define _setup_fancy_overwrite(start, name, target) \
    if (!_is_new_method_##name()) { \
        ERR("Method Is Not \"New\""); \
    } \
    static name##_t _original_for_##target = start; \
    static name##_t _helper_for_##target = _overwrite_helper_for_##name(target, _original_for_##target)

#define _update_references(from, to) \
    { \
        void *old_reference = (void *) from; \
        for (int i = 0; _all_method_symbols[i] != nullptr; i++) { \
            if (_all_method_symbols[i] == old_reference) { \
                _all_method_symbols[i] = (void *) to; \
            } \
        } \
    }

void _overwrite_calls(const char *file, int line, void *start, void *target);
#define overwrite_calls_manual(start, target) _overwrite_calls(__FILE__, __LINE__, start, target)
#define overwrite_calls(start, target) \
    { \
        _setup_fancy_overwrite(start, start, target); \
        overwrite_calls_manual((void *) start, (void *) _helper_for_##target); \
        _update_references(start, _helper_for_##target); \
    }

#define overwrite_virtual_calls(start, target) \
    { \
        _setup_fancy_overwrite(*start##_vtable_addr, start, target); \
        overwrite_calls_manual((void *) *start##_vtable_addr, (void *) _helper_for_##target); \
    }

void _overwrite_calls_within(const char *file, int line, void *from, void *to, void *start, void *target);
#define overwrite_calls_within(from, to, start, target) _overwrite_calls_within(__FILE__, __LINE__, from, to, start, target)

void *extract_from_bl_instruction(unsigned char *from);

void _overwrite(const char *file, int line, void *start, void *target);
#define overwrite(start, target) _overwrite(__FILE__, __LINE__, (void *) start, (void *) target)

void _patch(const char *file, int line, void *start, unsigned char patch[4]);
#define patch(start, patch) _patch(__FILE__, __LINE__, start, patch)

void _patch_address(const char *file, int line, void *start, void *target);
#define patch_address(start, target) _patch_address(__FILE__, __LINE__, (void *) start, (void *) target)

#endif

#ifdef __cplusplus
}
#endif
