#pragma once

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((visibility("internal"))) void _init_attack();
__attribute__((visibility("internal"))) void _init_misc();
__attribute__((visibility("internal"))) void _init_toggle();
__attribute__((visibility("internal"))) void _init_drop();
__attribute__((visibility("internal"))) void _init_crafting();

#ifdef __cplusplus
}
#endif
