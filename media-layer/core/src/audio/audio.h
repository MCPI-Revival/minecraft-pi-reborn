#pragma once

#include <al.h>

void _media_audio_delete_sources();

void _media_audio_init();
void _media_audio_cleanup();
int _media_audio_is_loaded();

ALuint _media_audio_get_buffer(const char *source, const char *name);
void _media_audio_delete_buffers();