#pragma once

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((visibility("internal"))) void _init_options_cpp();
__attribute__((visibility("internal"))) extern unsigned char *stored_options;

#ifdef __cplusplus
}
#endif
