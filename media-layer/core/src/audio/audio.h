#pragma once

#include <al.h>

MCPI_INTERNAL void _media_audio_delete_sources();

MCPI_INTERNAL void _media_audio_init();
MCPI_INTERNAL void _media_audio_cleanup();
MCPI_INTERNAL int _media_audio_is_loaded();

MCPI_INTERNAL ALuint _media_audio_get_buffer(const char *source, const char *name);
MCPI_INTERNAL void _media_audio_delete_buffers();