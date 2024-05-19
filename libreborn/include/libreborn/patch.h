#pragma once

// Patching Functions

#if defined(REBORN_HAS_PATCH_CODE) && defined(__cplusplus)

// Init
void reborn_init_patch();

// Replace Call Located At start With A Call To target
void _overwrite_call(const char *file, int line, void *start, void *target);
#define overwrite_call(...) \
    _overwrite_call(__FILE__, __LINE__, __VA_ARGS__)

// Make Sure Function Is Only Called Once
template <int>
static void _only_call_once() {
    static bool _has_run = false;
    if (_has_run) {
        ERR("\"Fancy\" overwrite*() Functions Can Only Be Called Once");
    }
    _has_run = true;
}

// Replace All Calls To Method start With target
void *_overwrite_calls_manual(const char *file, int line, void *start, void *target);
#define overwrite_calls_manual(...) \
    _overwrite_calls_manual(__FILE__, __LINE__, __VA_ARGS__)
template <int call_id, typename start_t, typename overwrite_t>
static void _overwrite_calls(const char *file, int line, start_t (*create_helper)(overwrite_t, start_t), start_t &start, overwrite_t target) {
    _only_call_once<call_id>();
    start_t helper = create_helper(target, start);
    start = (start_t) _overwrite_calls_manual(file, line, (void *) start, (void *) helper);
}
#define overwrite_calls(start, ...) \
    _overwrite_calls< \
        __COUNTER__, \
        start##_t, \
        __overwrite_##start##_t \
    >( \
        __FILE__, __LINE__, \
        __create_overwrite_helper_for_##start, \
        start, \
        __VA_ARGS__ \
    )

// Replace All Calls To Virtual Method start With target
template <int call_id, typename start_t, typename overwrite_t>
static void _overwrite_virtual_calls(const char *file, int line, start_t (*create_helper)(overwrite_t, start_t), bool (*is_overwritable)(), start_t start, overwrite_t target) {
    _only_call_once<call_id>();
    if (!is_overwritable()) {
        ERR("Virtual Method Is Not Overwritable");
    }
    start_t helper = create_helper(target, start);
    _overwrite_calls_manual(file, line, (void *) start, (void *) helper);
}
#define overwrite_virtual_calls(start, ...) \
    _overwrite_virtual_calls< \
        __COUNTER__, \
        start##_t, \
        __overwrite_##start##_t \
    >( \
        __FILE__, __LINE__, \
        __create_overwrite_helper_for_##start, \
        __is_overwritable_##start, \
        *start##_vtable_addr, \
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
        start##_t \
    >( \
        __FILE__, __LINE__, \
        from, to, \
        start, \
        __VA_ARGS__ \
    )

// Get Target Address From BL Instruction
void *extract_from_bl_instruction(unsigned char *from);

// Replace Method start With target
void _overwrite_manual(const char *file, int line, void *start, void *target);
#define overwrite_manual(...) \
    _overwrite(__FILE__, __LINE__, __VA_ARGS__)
template <typename start_t>
void _overwrite(const char *file, int line, start_t start, start_t target) {
    _overwrite_manual(file, line, (void *) start, (void *) target);
}
#define overwrite(start, ...) \
    _overwrite< \
        start##_t \
    >( \
        __FILE__, __LINE__, \
        start, \
        __VA_ARGS__ \
    )

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
        start##_t \
    >( \
        __FILE__, __LINE__, \
        start##_vtable_addr, \
        __VA_ARGS__ \
    )

#endif
