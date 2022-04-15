#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include <libreborn/libreborn.h>

#include "engine.h"
#include "file.h"

// Store Device
static ALCdevice *device = NULL;
static ALCcontext *context = NULL;

// Store State
static int is_loaded = 0;
int _media_audio_is_loaded() {
    return is_loaded;
}

// Init
void _media_audio_init() {
    // Open Device
    device = alcOpenDevice(NULL);
    if (!device) {
        WARN("Unable To Load Audio Engine");
        return;
    }

    // Create Context
    context = alcCreateContext(device, NULL);
    ALCenum err = alcGetError(device);
    if (err != ALC_NO_ERROR) {
        ERR("Unable To Open Audio Context: %s", alcGetString(device, err));
    }

    // Select Context
    alcMakeContextCurrent(context);
    err = alcGetError(device);
    if (err != ALC_NO_ERROR) {
        ERR("Unable To Select Audio Context: %s", alcGetString(device, err));
    }

    // Enable AL_SOURCE_DISTANCE_MODEL
    alEnable(AL_SOURCE_DISTANCE_MODEL);
    ALenum al_err = alGetError();
    if (al_err != AL_NO_ERROR) {
        ERR("Unable To Enable AL_SOURCE_DISTANCE_MODEL: %s", alGetString(al_err));
    }

    // Log
    INFO("Loaded Audio Engine");
    is_loaded = 1;
}

// De-Init
void _media_audio_cleanup() {
    if (_media_audio_is_loaded()) {
        // Delete Audio Buffers
        _media_audio_delete_buffers();

        // Deselect Context
        alcMakeContextCurrent(NULL);
        ALCenum err = alcGetError(device);
        if (err != ALC_NO_ERROR) {
            ERR("Unable To Deselect Audio Context: %s", alcGetString(device, err));
        }

        // Destroy Context
        alcDestroyContext(context);
        err = alcGetError(device);
        if (err != ALC_NO_ERROR) {
            ERR("Unable To Destroy Audio Context: %s", alcGetString(device, err));
        }

        // Close Device
        alcCloseDevice(device);
        err = alcGetError(device);
        if (err != ALC_NO_ERROR) {
            ERR("Unable To Close Audio Device: %s", alcGetString(device, err));
        }

        // Log
        INFO("Unloaded Audio Engine");
    }
}
