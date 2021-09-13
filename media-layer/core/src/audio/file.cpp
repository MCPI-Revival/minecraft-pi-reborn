#include <unordered_map>
#include <string>
#include <functional>
#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <sys/mman.h>
#include <elf.h>

#include <AL/al.h>

#include <libreborn/libreborn.h>

#include "file.h"
#include "engine.h"

// Load Symbol From ELF File
static void load_symbol(const char *source, const char *name, std::function<void(unsigned char *, uint32_t)> callback) {
    // File Data
    FILE *file_obj = NULL;
    unsigned char *file_map = NULL;
    long int file_size = 0;

    // Code
    {
        // Load Main Binary
        file_obj = fopen(source, "rb");

        // Verify Binary
        if (!file_obj) {
            WARN("Unable To Open: %s", source);
            goto end;
        }

        // Get File Size
        fseek(file_obj, 0L, SEEK_END);
        file_size = ftell(file_obj);
        fseek(file_obj, 0L, SEEK_SET);

        // Map File To Pointer
        file_map = (unsigned char *) mmap(0, file_size, PROT_READ, MAP_PRIVATE, fileno(file_obj), 0);

        // Check ELF Magic
        if (file_map[EI_MAG0] != ELFMAG0 || file_map[EI_MAG1] != ELFMAG1 || file_map[EI_MAG2] != ELFMAG2 || file_map[EI_MAG3] != ELFMAG3) {
            WARN("Not An ELF File: %s", source);
            goto end;
        }
        if (file_map[EI_CLASS] != ELFCLASS32) {
            WARN("ELF File Isn't 32-Bit: %s", source);
            goto end;
        }
        if (file_map[EI_DATA] != ELFDATA2LSB) {
            WARN("ELF File Isn't Little-Endian: %s", source);
            goto end;
        }

        // Parse ELF
        Elf32_Ehdr *elf_header = (Elf32_Ehdr *) file_map;
        Elf32_Shdr *elf_section_headers = (Elf32_Shdr *) (file_map + elf_header->e_shoff);
        int elf_section_header_count = elf_header->e_shnum;

        // Locate Section Names
        Elf32_Shdr elf_shstrtab = elf_section_headers[elf_header->e_shstrndx];
        unsigned char *elf_shstrtab_p = file_map + elf_shstrtab.sh_offset;

        // Locate String Table
        unsigned char *elf_strtab_p = NULL;
        for (int i = 0; i < elf_section_header_count; ++i) {
            Elf32_Shdr header = elf_section_headers[i];
            // Check Section Type
            if (header.sh_type == SHT_STRTAB) {
                // Check Section Name
                char *section_name = (char *) (elf_shstrtab_p + header.sh_name);
                if (strcmp(section_name, ".dynstr") == 0) {
                    // Found
                    elf_strtab_p = file_map + header.sh_offset;
                    break;
                }
            }
        }
        if (elf_strtab_p == NULL) {
            WARN("Unable To Find String Table In: %s", source);
            goto end;
        }

        // Locate Symbol Tables
        Elf32_Sym *symbol = NULL;
        for (int i = 0; i < elf_section_header_count; ++i) {
            // Exit Loop If Finished
            if (symbol != NULL) {
                break;
            }
            // Get Section Header
            Elf32_Shdr header = elf_section_headers[i];
            // Check Section Type
            if (header.sh_type == SHT_DYNSYM) {
                // Symbol Table
                Elf32_Sym *table = (Elf32_Sym *) (file_map + header.sh_offset);
                for (int j = 0; (j * sizeof (Elf32_Sym)) < header.sh_size; j++) {
                    // Check Symbol Name
                    char *symbol_name = (char *) (elf_strtab_p + table[j].st_name);
                    if (strcmp(symbol_name, name) == 0) {
                        // Found
                        symbol = &table[j];
                        break;
                    }
                }
            }
        }

        // Check Symbol
        if (symbol != NULL) {
            // Convert Virtual Address To File Offset
            Elf32_Shdr symbol_section_header = elf_section_headers[symbol->st_shndx];
            int vaddr_to_offset = -symbol_section_header.sh_addr + symbol_section_header.sh_offset;
            Elf32_Off symbol_offset = symbol->st_value + vaddr_to_offset;
            // Access Symbol
            unsigned char *value = file_map + symbol_offset;
            uint32_t size = symbol->st_size;
            callback(value, size);
        } else {
            // Unable To Find Symbol
            WARN("Unable To Find Symbol: %s", name);
        }
    }

 end:
    // Unmap And Close File
    if (file_map != NULL) {
        munmap(file_map, file_size);
    }
    if (file_obj != NULL) {
        fclose(file_obj);
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
    load_symbol(source, name, [name, &buffer](unsigned char *symbol, uint32_t size) {
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
        unsigned char *data = symbol + sizeof (audio_metadata);

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
static std::unordered_map<std::string, ALuint> &get_buffers() {
    static std::unordered_map<std::string, ALuint> buffers;
    return buffers;
}

// Get Buffer For Sound
ALuint _media_audio_get_buffer(const char *source, const char *name) {
    // Check
    if (_media_audio_is_loaded()) {
        if (get_buffers().count(name) > 0) {
            // Return
            return get_buffers()[name];
        } else {
            // Load And Return
            get_buffers()[name] = load_sound(source, name);
            return _media_audio_get_buffer(source, name);
        }
    } else {
        ERR("%s", "Audio Engine Isn't Loaded");
    }
}

// Delete Buffers
void _media_audio_delete_buffers() {
    if (_media_audio_is_loaded()) {
        for (auto it : get_buffers()) {
            if (it.second && alIsBuffer(it.second)) {
                alDeleteBuffers(1, &it.second);
            }
        }
    }
    get_buffers().clear();
}
