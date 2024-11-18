#pragma once

#include <AL/al.h>

__attribute__((visibility("internal"))) void _media_audio_init();
__attribute__((visibility("internal"))) void _media_audio_cleanup();
__attribute__((visibility("internal"))) int _media_audio_is_loaded();