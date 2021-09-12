#pragma once

#include <AL/al.h>

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((visibility("internal"))) void _media_audio_init();
__attribute__((visibility("internal"))) void _media_audio_cleanup();
__attribute__((visibility("internal"))) int _media_audio_is_loaded();

#ifdef __cplusplus
}
#endif
