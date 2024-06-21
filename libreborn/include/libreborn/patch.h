#pragma once

// Patching Functions

#if defined(REBORN_HAS_PATCH_CODE) && defined(__cplusplus)

#include <string>
#include <functional>

// Init
void reborn_init_patch();

// Replace Call Located At start With A Call To target
void _overwrite_call(const char *file, int line, void *start, void *target);
#define overwrite_call(...) \
    _overwrite_call(__FILE__, __LINE__, __VA_ARGS__)

// Replace All Calls To Method start With target
void *_overwrite_calls_manual(const char *file, int line, void *start, void *target);
#define overwrite_calls_manual(...) \
    _overwrite_calls_manual(__FILE__, __LINE__, __VA_ARGS__)
template <typename overwrite_t>
void _overwrite_calls(const char *file, int line, std::string (*set_overwrite)(const overwrite_t &, const std::function<void *(void *, void *)> &), const overwrite_t &target) {
    std::string ret = set_overwrite(target, [&file, &line](void *start, void *target2) {
        return _overwrite_calls_manual(file, line, start, target2);
    });
    if (!ret.empty()) {
        ERR("%s", ret.c_str());
    }
}
#define overwrite_calls(start, ...) \
    _overwrite_calls< \
        __overwrite_##start##_t \
    >( \
        __FILE__, __LINE__, \
        __set_overwrite_for_##start, \
        __VA_ARGS__ \
    )

// Replace All Calls To start With target Within [to, from)
void _overwrite_calls_within_manual(const char *file, int line, void *from, void *to, void *start, void *target);
#define overwrite_calls_within_manual(...) \
    _overwrite_calls_within(__FILE__, __LINE__, __VA_ARGS__)
template <typename start_t>
void _overwrite_calls_within(const char *file, int line, void *from, void *to, start_t start, start_t target) {
    _overwrite_calls_within_manual(file, line, from, to, (void *) start, (void *) target);
}
#define overwrite_calls_within(from, to, start, ...) \
    _overwrite_calls_within< \
        __raw_##start##_t \
    >( \
        __FILE__, __LINE__, \
        from, to, \
        start, \
        __VA_ARGS__ \
    )

// Get Target Address From BL Instruction
void *extract_from_bl_instruction(unsigned char *from);

// Patch Instruction
void _patch(const char *file, int line, void *start, unsigned char patch[4]);
#define patch(...) \
    _patch(__FILE__, __LINE__, __VA_ARGS__)

// Patch 4 Bytes Of Data
void _patch_address(const char *file, int line, void *start, void *target);
#define patch_address(...) \
    _patch_address(__FILE__, __LINE__, __VA_ARGS__)

// Patch VTable Entry
// This does not affect sub-classes.
template <typename start_t>
void _patch_vtable(const char *file, int line, start_t *start, start_t target) {
    _patch_address(file, line, (void *) start, (void *) target);
}
#define patch_vtable(start, ...) \
    _patch_vtable< \
        __raw_##start##_t \
    >( \
        __FILE__, __LINE__, \
        start##_vtable_addr, \
        __VA_ARGS__ \
    )

#endif
