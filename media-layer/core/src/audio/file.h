#pragma once

#include <AL/al.h>

__attribute__((visibility("internal"))) ALuint _media_audio_get_buffer(const char *source, const char *name);
__attribute__((visibility("internal"))) void _media_audio_delete_buffers();
