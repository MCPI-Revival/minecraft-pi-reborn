#include <libreborn/patch.h>

#include <mods/display-lists/display-lists.h>

#include "thread.h"

// Allow Creating BiomeSource Without Level
static thread_local std::optional<int> forced_biome_source_seed;
static int BiomeSource_constructor_Level_getSeed_injection(Level *self) {
    if (forced_biome_source_seed.has_value()) {
        return forced_biome_source_seed.value();
    } else {
        return self->getSeed();
    }
}

// Access Biome Data From Chunk Building Thread
static thread_local BiomeSource *biome_source = nullptr;
void _create_biome_source(const int seed) {
    forced_biome_source_seed = seed;
    biome_source = BiomeSource::allocate();
    biome_source->constructor(nullptr);
}
BiomeSource *get_biome_source_on_chunk_rebuild_thread() {
    return biome_source;
}
void _free_biome_source() {
    if (biome_source) {
        biome_source->destructor_deleting();
        biome_source = nullptr;
    }
}

// Init
void _init_threaded_biome_source() {
    for (const uint32_t addr : {0xaf79c, 0xaf7c0, 0xaf7d8}) {
        overwrite_call((void *) addr, Level_getSeed, BiomeSource_constructor_Level_getSeed_injection);
    }
}