#include <string>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>
#include <media-layer/audio.h>

#include "sound-internal.h"
#include <mods/feature/feature.h>
#include <mods/override/override.h>
#include <mods/init/init.h>

// Resolve Source File Path
#define SOURCE_FILE_BASE "data/libminecraftpe.so"
extern std::string info_sound_data_state;
std::string _sound_get_source_file() {
    static bool source_loaded = false;
    static std::string source;

    // Check
    if (source_loaded) {
        // Already Resolved
        return source;
    } else {
        // Resolve

        // Get Path
        const std::string path = SOURCE_FILE_BASE;
        const std::string full_path = override_get_path(path);

        // Check If Sound Exists
        if (access(full_path.c_str(), F_OK) == -1) {
            // Fail
            WARN("Missing Audio Source File: %s", path.c_str());
            source = "";
            info_sound_data_state = "Missing";
        } else {
            // Set
            source = full_path;
            info_sound_data_state = "Loaded";
        }

        // Mark As Loaded
        source_loaded = true;

        // Return
        return _sound_get_source_file();
    }
}

// Play Sound
// The pitch value is unsued because it causes glitchy sounds, it is seemingly unused in MCPE as well.
static void play(const std::string &name, const float x, const float y, const float z, const float volume, float pitch, const bool is_ui) {
    const std::string source = _sound_get_source_file();
    const std::string resolved_name = _sound_pick(name);
    if (pitch < 0.01f) {
        pitch = 1;
    }
    if (source.size() > 0 && resolved_name.size() > 0) {
        media_audio_play(source.c_str(), resolved_name.c_str(), x, y, z, pitch, volume, is_ui);
    }
}
static void SoundEngine_playUI_injection(__attribute__((unused)) SoundEngine_playUI_t original, __attribute__((unused)) SoundEngine *sound_engine, const std::string &name, const float volume, const float pitch) {
    play(name, 0, 0, 0, volume, pitch, true);
}
static void SoundEngine_play_injection(__attribute__((unused)) SoundEngine_play_t original, __attribute__((unused)) SoundEngine *sound_engine, const std::string &name, const float x, const float y, const float z, const float volume, const float pitch) {
    play(name, x, y, z, volume, pitch, false);
}

// Refresh Data
static void SoundEngine_update_injection(__attribute__((unused)) SoundEngine_update_t original, SoundEngine *sound_engine, Mob *listener_mob, __attribute__((unused)) float listener_angle) {
    // Variables
    static float volume = 0;
    static float x = 0;
    static float y = 0;
    static float z = 0;
    static float yaw = 0;

    // SoundEngine Properties
    Options *options = sound_engine->options;

    // Volume
    const int32_t sound_enabled = options->sound;
    volume = sound_enabled ? 1 : 0;

    // Position And Rotation
    if (listener_mob != nullptr) {
        // Values
        x = listener_mob->x;
        y = listener_mob->y;
        z = listener_mob->z;
        yaw = listener_mob->yaw;
    }

    // Log
    media_audio_update(volume, x, y, z, yaw);
}

// Resolve All Sounds On Init
// SoundEngine::init Is Called After The Audio Engine Has Been Loaded
static void SoundEngine_init_injection(SoundEngine_init_t original, SoundEngine *sound_engine, Minecraft *minecraft, Options *options) {
    // Call Original Method
    original(sound_engine, minecraft, options);

    // Resolve Sounds
    _sound_resolve_all();
}

// Init
void init_sound() {
    // Implement Sound Engine
    if (feature_has("Implement Sound Engine", server_disabled)) {
        overwrite_calls(SoundEngine_playUI, SoundEngine_playUI_injection);
        overwrite_calls(SoundEngine_play, SoundEngine_play_injection);
        overwrite_calls(SoundEngine_update, SoundEngine_update_injection);
        overwrite_calls(SoundEngine_init, SoundEngine_init_injection);
    }
}
