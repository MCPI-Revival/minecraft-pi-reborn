---
gitea: none
include_toc: true
---

# Advanced Modding
This chapter covers precise and low-level modding.

All the following functions may easily cause conflicts with other mods as they do not support layering.

This chapter assumes basic knowledge of C++ and ARM32 Assembly.

## `patch`(`_address`)
These functions will patch a single instruction.

### Example
```c++
// Inside Init Function
unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
patch((void *) 0xabcde, nop_patch);
patch_address((void *) 0xedcba, (void *) "Hello!");
```

## `overwrite_call`
This function will patch a single function call.

The naming convention for replacement functions is `<parent function>_<target function>_injection`.

### Example
```c++
// The Replacement Function
static void TripodCamera_tick_Level_addParticle_injection(Level *level, const std::string &particle, const float x, const float y, const float z, const float deltaX, const float deltaY, const float deltaZ, const int count) {
    // Call Original Method
    level->addParticle(particle, x, y, z, deltaX, deltaY, deltaZ, count);
}
// Inside Init Function
overwrite_call((void *) 0x87dc4, Level_addParticle, TripodCamera_tick_Level_addParticle_injection);
```

## `overwrite_calls_within`
This will overwrite all function calls to a specified method within the provided `[from, to)` address range.

### Example
```c++
// The Replacement Function
static void Level_addParticle_injection(Level *level, const std::string &particle, const float x, const float y, const float z, const float deltaX, const float deltaY, const float deltaZ, const int count) {
    // Call Original Method
    level->addParticle(particle, x, y, z, deltaX, deltaY, deltaZ, count);
}
// Inside Init Function
overwrite_calls_within((void *) 0xabcde, (void *) 0xedcba, Level_addParticle, Level_addParticle_injection);
```

## `extract_from_bl_instruction`
This retrieves the target of the specified jump instruction.

### Example
```c++
void *addr = extract_from_bl_instruction((unsigned char *) 0xabcde);
```

## `overwrite_*_manual`
These are type-unsafe versions of the other game patching functions.
They also allow targeting functions without method objects.

### Example
```c++
// Inside Init Function
overwrite_call_manual((void *) 0xabcde, (void *) func_1);
overwrite_calls_manual((void *) 0xedcba, (void *) func_1);
overwrite_calls_within_manual((void *) 0xabcde, (void *) 0xedcba, (void *) func_1, (void *) func_2);
```