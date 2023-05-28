#include <unordered_map>
#include <string>
#include <functional>

#include <AL/al.h>

#include <LIEF/ELF.hpp>

#include <libreborn/libreborn.h>

#include "file.h"
#include "engine.h"

// Load Symbol From ELF File
static void load_symbol(const char *source, const char *name, std::function<void(const unsigned char *, uint32_t)> callback) {
    static std::unique_ptr<LIEF::ELF::Binary> binary = NULL;
    if (binary == NULL) {
        binary = LIEF::ELF::Parser::parse(source);
    }
    const LIEF::ELF::Symbol *symbol = binary->get_dynamic_symbol(name);
    if (symbol != NULL) {
        LIEF::span<const uint8_t> data = binary->get_content_from_virtual_address(symbol->value(), symbol->size(), LIEF::Binary::VA_TYPES::VA);
        callback(data.data(), data.size());
    } else {
        WARN("Unable To Find Symbol: %s", name);
    }
}

// Audio Metadata
struct audio_metadata {
    int32_t channels;
    int32_t frame_size;
    int32_t sample_rate;
    int32_t frames;
};

// Load Sound Symbol Into ALunit
static ALuint load_sound(const char *source, const char *name) {
    // Check OpenAL
    if (!_media_audio_is_loaded()) {
        return 0;
    }

    // Store Result
    ALuint buffer = 0;

    // Load Symbol
    load_symbol(source, name, [name, &buffer](const unsigned char *symbol, uint32_t size) {
        // Load Metadata
        if (size < sizeof (audio_metadata)) {
            WARN("Symbol Too Small To Contain Audio Metadata: %s", name);
            return;
        }
        audio_metadata *meta = (audio_metadata *) symbol;

        // Check Frame Size
        if (meta->frame_size != 1 && meta->frame_size != 2) {
            WARN("Unsupported Frame Size: %s: %i", name, meta->frame_size);
            return;
        }

        // Get Audio Format
        ALenum format = AL_NONE;
        if (meta->channels == 1) {
            format = meta->frame_size == 2 ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8;
        } else if (meta->channels == 2) {
            format = meta->frame_size == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_STEREO8;
        } else {
            WARN("Unsupported Channel Count: %s: %i", name, meta->channels);
            return;
        }

        // Load Data
        int remaining_size = size - sizeof (audio_metadata);
        int data_size = meta->channels * meta->frames * meta->frame_size;
        if (remaining_size < data_size) {
            WARN("Symbol Too Small To Contain Specified Audio Data: %s", name);
            return;
        }
        const unsigned char *data = symbol + sizeof (audio_metadata);

        // Create Buffer
        alGenBuffers(1, &buffer);
        alBufferData(buffer, format, data, data_size, meta->sample_rate);

        // Check OpenAL Error
        ALenum err = alGetError();
        if (err != AL_NO_ERROR) {
            WARN("Unable To Store Audio Buffer: %s", alGetString(err));
            if (buffer && alIsBuffer(buffer)) {
                alDeleteBuffers(1, &buffer);
            }
            buffer = 0;
        }
    });

    // Return
    return buffer;
}

// Store Buffers
static std::unordered_map<std::string, ALuint> buffers;

// Get Buffer For Sound
ALuint _media_audio_get_buffer(const char *source, const char *name) {
    // Check
    if (_media_audio_is_loaded()) {
        if (buffers.count(name) > 0) {
            // Return
            return buffers[name];
        } else {
            // Load And Return
            buffers[name] = load_sound(source, name);
            return _media_audio_get_buffer(source, name);
        }
    } else {
        ERR("Audio Engine Isn't Loaded");
    }
}

// Delete Buffers
void _media_audio_delete_buffers() {
    if (_media_audio_is_loaded()) {
        for (auto &it : buffers) {
            if (it.second && alIsBuffer(it.second)) {
                alDeleteBuffers(1, &it.second);
            }
        }
    }
    buffers.clear();
}
