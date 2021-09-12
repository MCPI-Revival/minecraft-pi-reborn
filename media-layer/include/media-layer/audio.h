#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void media_audio_update(float volume, float x, float y, float z, float yaw);
void media_audio_play(const char *source, const char *name, float x, float y, float z, float pitch, float volume, int is_ui);

#ifdef __cplusplus
}
#endif
