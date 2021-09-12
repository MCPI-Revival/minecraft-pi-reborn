#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "log.h"
#include "util.h"
#include "string.h"
#include "exec.h"
#include "elf.h"

#ifdef REBORN_HAS_COMPILED_CODE

// Patching Functions

void _overwrite_call(const char *file, int line, void *start, void *target);
#define overwrite_call(start, target) _overwrite_call(__FILE__, __LINE__, start, target);

void _overwrite_calls(const char *file, int line, void *start, void *target);
#define overwrite_calls(start, target) _overwrite_calls(__FILE__, __LINE__, start, target);

void _overwrite(const char *file, int line, void *start, void *target);
#define overwrite(start, target) _overwrite(__FILE__, __LINE__, start, target);

void _patch(const char *file, int line, void *start, unsigned char patch[4]);
#define patch(start, patch) _patch(__FILE__, __LINE__, start, patch);

void _patch_address(const char *file, int line, void *start, void *target);
#define patch_address(start, target) _patch_address(__FILE__, __LINE__, start, target);

#endif // #ifdef __arm__

#ifdef __cplusplus
}
#endif
