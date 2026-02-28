#include <libreborn/patch.h>

#include "internal.h"

// Prevent Terrain Generation When Accessing Chunks
bool _inhibit_terrain_generation = false;
template <typename Self, typename... Args>
struct block_terrain_generation {
    template <void (Self::*func)(Args...)>
    static void injection(Self *self, Args... args) {
        if (_inhibit_terrain_generation) {
            return;
        }
        // Call Original Method
        (self->*func)(std::forward<Args>(args)...);
    }
};
static Biome **RandomLevelSource_getChunk_BiomeSource_getBiomeBlock_injection(BiomeSource *self, const int x, const int z, const int param_1, const int param_2) {
    if (_inhibit_terrain_generation) {
        return nullptr;
    }
    // Call Original Method
    return self->getBiomeBlock(x, z, param_1, param_2);
}
static LevelChunk *RandomLevelSource_getChunk_injection(RandomLevelSource_getChunk_t original, RandomLevelSource *self, const int chunk_x, const int chunk_z) {
    // Call Original Method
    LevelChunk *ret = original(self, chunk_x, chunk_z);
    // Mark Terrain As Generated
    if (_inhibit_terrain_generation) {
        ret->done_generating = true;
    }
    // Return
    return ret;
}

// Init
void _init_multiplayer_loading_terrain() {
    // Allow Blocking Terrain Generation
    overwrite_call((void *) 0xb488c, RandomLevelSource_buildSurface, block_terrain_generation<RandomLevelSource, int, int, uchar *, Biome **>::injection<&RandomLevelSource::buildSurface>);
    overwrite_call((void *) 0xb4874, RandomLevelSource_prepareHeights, block_terrain_generation<RandomLevelSource, int, int, uchar *, void *, float *>::injection<&RandomLevelSource::prepareHeights>);
    overwrite_call((void *) 0xb489c, LevelChunk_recalcHeightmap, block_terrain_generation<LevelChunk>::injection<&LevelChunk::recalcHeightmap>);
    overwrite_call((void *) 0xb4844, BiomeSource_getBiomeBlock, RandomLevelSource_getChunk_BiomeSource_getBiomeBlock_injection);
    overwrite_calls(RandomLevelSource_getChunk, RandomLevelSource_getChunk_injection);
}