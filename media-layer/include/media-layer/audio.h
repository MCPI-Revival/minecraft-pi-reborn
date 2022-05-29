#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MCPI_HEADLESS_MODE
void media_audio_update(float volume, float x, float y, float z, float yaw);
void media_audio_play(const char *source, const char *name, float x, float y, float z, float pitch, float volume, int is_ui);
#else
static inline void media_audio_update(__attribute__((unused)) float volume, __attribute__((unused)) float x, __attribute__((unused)) float y, __attribute__((unused)) float z, __attribute__((unused)) float yaw) {
}
static inline void media_audio_play(__attribute__((unused)) const char *source, __attribute__((unused)) const char *name, __attribute__((unused)) float x, __attribute__((unused)) float y, __attribute__((unused)) float z, __attribute__((unused)) float pitch, __attribute__((unused)) float volume, __attribute__((unused)) int is_ui) {
}
#endif

#ifdef __cplusplus
}
#endif
