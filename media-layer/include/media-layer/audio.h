#pragma once

extern "C" {
MCPI_MEDIA_LAYER_CORE_PUBLIC void media_audio_update(float volume, float x, float y, float z, float yaw);
MCPI_MEDIA_LAYER_CORE_PUBLIC void media_audio_play(const char *source, const char *name, float x, float y, float z, float pitch, float volume, int is_ui);
}
