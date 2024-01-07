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
        char *path = strdup(SOURCE_FILE_BASE);
        ALLOC_CHECK(path);

        // Handle Overrides
        char *overridden_full_path = override_get_path(path);
        if (overridden_full_path != NULL) {
            free(path);
            path = overridden_full_path;
        }

        // Check If Sound Exists
        if (access(path, F_OK) == -1) {
            // Fail
            WARN("Audio Source File Doesn't Exist: " SOURCE_FILE_BASE);
            source.assign("");
        } else {
            // Set
            source.assign(path);
        }

        // Free
        free(path);

        // Mark As Loaded
        source_loaded = true;

        // Return
        return _sound_get_source_file();
    }
}

// Play Sound
// The pitch value is unsued because it causes glitchy sounds, it is seemingly unused in MCPE as well.
static void play(std::string name, float x, float y, float z, float volume, float pitch, bool is_ui) {
    std::string source = _sound_get_source_file();
    std::string resolved_name = _sound_pick(name);
    if (pitch < 0.01f) {
        pitch = 1;
    }
    if (source.size() > 0 && resolved_name.size() > 0) {
        media_audio_play(source.c_str(), resolved_name.c_str(), x, y, z, pitch, volume, is_ui);
    }
}
static void SoundEngine_playUI_injection(__attribute__((unused)) unsigned char *sound_engine, std::string const& name, float volume, float pitch) {
    play(name, 0, 0, 0, volume, pitch, true);
}
static void SoundEngine_play_injection(__attribute__((unused)) unsigned char *sound_engine, std::string const& name, float x, float y, float z, float volume, float pitch) {
    play(name, x, y, z, volume, pitch, false);
}

// Refresh Data
static void SoundEngine_update_injection(SoundEngine *sound_engine, Mob *listener_mob, __attribute__((unused)) float listener_angle) {
    // Variables
    static float volume = 0;
    static float x = 0;
    static float y = 0;
    static float z = 0;
    static float yaw = 0;

    // SoundEngine Properties
    Options *options = sound_engine->options;

    // Volume
    int32_t sound_enabled = options->sound;
    volume = sound_enabled ? 1 : 0;

    // Position And Rotation
    if (listener_mob != NULL) {
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
static void SoundEngine_init_injection(SoundEngine *sound_engine, Minecraft *minecraft, Options *options) {
    // Call Original Method
    SoundEngine_init(sound_engine, minecraft, options);

    // Resolve Sounds
    _sound_resolve_all();
}

// Init
void init_sound() {
    // Implement Sound Engine
    if (feature_has("Implement Sound Engine", server_disabled)) {
        overwrite((void *) SoundEngine_playUI, (void *) SoundEngine_playUI_injection);
        overwrite((void *) SoundEngine_play, (void *) SoundEngine_play_injection);
        overwrite((void *) SoundEngine_update, (void *) SoundEngine_update_injection);
        overwrite_calls((void *) SoundEngine_init, (void *) SoundEngine_init_injection);
    }
}
