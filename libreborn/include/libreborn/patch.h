#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Patching Functions

#ifdef REBORN_HAS_PATCH_CODE

void reborn_init_patch();

void _overwrite_call(const char *file, int line, void *start, void *target);
#define overwrite_call(start, target) _overwrite_call(__FILE__, __LINE__, start, target);

void _overwrite_calls(const char *file, int line, void *start, void *target);
#define overwrite_calls(start, target) _overwrite_calls(__FILE__, __LINE__, start, target);

void _overwrite_calls_within(const char *file, int line, void *from, void *to, void *start, void *target);
#define overwrite_calls_within(from, to, start, target) _overwrite_calls_within(__FILE__, __LINE__, from, to, start, target);

void *extract_from_bl_instruction(unsigned char *from);

void _overwrite(const char *file, int line, void *start, void *target);
#define overwrite(start, target) _overwrite(__FILE__, __LINE__, start, target);

void _patch(const char *file, int line, void *start, unsigned char patch[4]);
#define patch(start, patch) _patch(__FILE__, __LINE__, start, patch);

void _patch_address(const char *file, int line, void *start, void *target);
#define patch_address(start, target) _patch_address(__FILE__, __LINE__, start, target);

#endif

#ifdef __cplusplus
}
#endif
