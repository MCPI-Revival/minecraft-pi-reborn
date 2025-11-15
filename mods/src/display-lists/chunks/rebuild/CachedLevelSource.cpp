#include <bit>

#include "CachedLevelSource.h"

// Helpful Macros
#define block data[x - x0][z - z0][y - y0]
static constexpr unsigned int level_chunk_size = CachedLevelSource::LEVEL_CHUNK_SIZE;
static constexpr unsigned int chunk_shift_factor = std::bit_width(level_chunk_size) - 1;
#define to_chunk_pos(x) ((x) >> chunk_shift_factor)
#define to_local_pos(x) ((x) & (level_chunk_size - 1))
#define is_in_chunk(extra) \
    (x >= (x0 + (extra)) && x < (x1 - (extra)) \
    && y >= (y0 + (extra)) && y < (y1 - (extra)) && \
    z >= (z0 + (extra)) && z < (z1 - (extra)))
#define op_dir(op) x op dir[0]; y op dir[1]; z op dir[2]
#define add_dir op_dir(+=)
#define sub_dir op_dir(-=)

// Define cached region
// and copy tiles.
void CachedLevelSource::_prepare_cache(Level *level, const int x0_, const int y0_, const int z0_, const int x1_, const int y1_, const int z1_) {
    x0 = x0_ - BORDER;
    y0 = y0_ - BORDER;
    z0 = z0_ - BORDER;
    x1 = x1_ + BORDER;
    y1 = y1_ + BORDER;
    z1 = z1_ + BORDER;
    _copy_chunks(level);
}
void CachedLevelSource::_cache() {
    _copy_tiles();
    _check_if_should_render();
}

// Copy Chunks
LevelChunk *CachedLevelSource::_clone(LevelChunk *chunk) {
    if (chunk->isEmpty()) {
        EmptyLevelChunk *out = EmptyLevelChunk::allocate();
        ((LevelChunk *) out)->constructor(nullptr, nullptr, 0, 0);
        out->vtable = EmptyLevelChunk::VTable::base;
        out->prevent_save = true;
        return (LevelChunk *) out;
    } else {
        uchar *blocks = new uchar[chunk->blocks_length];
        memcpy(blocks, chunk->blocks, chunk->blocks_length);
        LevelChunk *out = LevelChunk::allocate();
        out->constructor(nullptr, blocks, 0, 0);
        memcpy(out->data.data, chunk->data.data, chunk->data.length);
        memcpy(out->light_sky.data, chunk->light_sky.data, chunk->light_sky.length);
        memcpy(out->light_block.data, chunk->light_block.data, chunk->light_block.length);
        return out;
    }
}
void CachedLevelSource::_copy_chunks(Level *level) {
    // Copy Chunks
    chunk_x0 = to_chunk_pos(x0);
    const int chunk_x1 = to_chunk_pos(x1 - 1);
    chunk_z0 = to_chunk_pos(z0);
    const int chunk_z1 = to_chunk_pos(z1 - 1);
    chunks_x = chunk_x1 - chunk_x0 + 1;
    chunks_z = chunk_z1 - chunk_z0 + 1;
    for (int chunk_x = 0; chunk_x < chunks_x; chunk_x++) {
        for (int chunk_z = 0; chunk_z < chunks_z; chunk_z++) {
            chunks[chunk_x][chunk_z] = _clone(level->getChunk(chunk_x + chunk_x0, chunk_z + chunk_z0));
        }
    }

    // Copy Level Information
    for (int i = 0; i <= MAX_BRIGHTNESS; i++) {
        light_ramp[i] = level->dimension->light_ramp[i];
    }
    level_field_6c = level->field_6c;
}

// Copy Tile Information From Level
void CachedLevelSource::_copy_tiles() {
    // Raw Brightness Constants
    // See: Level::getRawBrightness
    constexpr int under_world_brightness = 0;
    constexpr int max_brightness = 15;
    const int over_world_brightness = std::max(max_brightness - level_field_6c, under_world_brightness);

    // Cache Tile Information
    touched_sky = false;
    for (int x = x0; x < x1; x++) {
        const int chunk_x = to_chunk_pos(x);
        for (int z = z0; z < z1; z++) {
            const int chunk_z = to_chunk_pos(z);
            LevelChunk *chunk = chunks[chunk_x - chunk_x0][chunk_z - chunk_z0];
            for (int y = y0; y < y1; y++) {
                const bool over_world = y > MAX_Y;
                const bool valid_y = y >= MIN_Y && !over_world;
                Data &obj = block;
                if (valid_y) {
#define xyz to_local_pos(x), y, to_local_pos(z)
                    obj.id = chunk->getTile(xyz);
                    obj.data = chunk->getData(xyz);
                    obj.raw_brightness = _get_raw_brightness(chunk, xyz);
#undef xyz
                } else {
                    obj.id = 0;
                    obj.data = 0;
                    obj.raw_brightness = over_world ? over_world_brightness : under_world_brightness;
                }
                obj.is_empty = obj.id <= 0;
                obj.material = obj.is_empty ? Material::air : Tile::tiles[obj.id]->material;
                obj.should_render = false;
            }
        }
    }

    // Delete Chunks
    // They are no longer needed.
    for (int chunk_x = 0; chunk_x < chunks_x; chunk_x++) {
        for (int chunk_z = 0; chunk_z < chunks_z; chunk_z++) {
            LevelChunk *chunk = chunks[chunk_x][chunk_z];
            chunk->deleteBlockData();
            chunk->destructor_deleting();
        }
    }

    // Cache Brightness
    // This is a separate loop because brightness
    // calculations may require neighbor tiles.
    for (int x = x0; x < x1; x++) {
        for (int z = z0; z < z1; z++) {
            for (int y = y0; y < y1; y++) {
                // See: Level::getBrightness
                block.brightness = light_ramp[_get_raw_brightness(x, y, z, true)];
            }
        }
    }
}

// Check If Tiles Should Be Rendered
void CachedLevelSource::_check_if_should_render() {
    should_render = false;
    for (int x = x0; x < x1; x++) {
        for (int z = z0; z < z1; z++) {
            for (int y = y0; y < y1; y++) {
                const Data &obj = block;
                // Check If Block Is Solid
                const bool is_solid = !obj.is_empty && Tile::tiles[obj.id]->isSolidRender();
                if (is_solid) {
                    continue;
                }
                // Block Is Non-Solid
                // Mark Neighbors As Visible
                constexpr int directions[][3] = {
                    {0, 0, 0},
                    {0, BORDER, 0},
                    {0, -BORDER, 0},
                    {BORDER, 0, 0},
                    {-BORDER, 0, 0},
                    {0, 0, BORDER},
                    {0, 0, -BORDER}
                };
                for (const int *dir : directions) {
                    add_dir;
                    if (is_in_chunk(0)) {
                        Data &obj2 = block;
                        obj2.should_render = !obj2.is_empty;
                        if (obj2.should_render && is_in_chunk(BORDER)) {
                            should_render = true;
                        }
                    }
                    sub_dir;
                }
            }
        }
    }
}

// Calculate The Adjusted Raw Brightness Of A Tile
int CachedLevelSource::_get_raw_brightness(LevelChunk *chunk, const int x, const int y, const int z) {
    // See LevelChunk::getRawBrightness
    // This function was re-implemented to avoid modifying
    // the LevelChunk::touchedSky global variable.
    int light_sky = chunk->getBrightness(LightLayer::Sky, x, y, z);
    if (light_sky > 0) {
        touched_sky = true;
    }
    light_sky -= level_field_6c;
    const int light_block = chunk->getBrightness(LightLayer::Block, x, y, z);
    return std::max(light_sky, light_block);
}
int CachedLevelSource::_get_raw_brightness(int x, int y, int z, const bool param_1) const {
    // See: Level::getRawBrightness
    const Data &obj = block;
    if (param_1) {
        const int id = obj.id;
        if (id == Tile::stoneSlabHalf->id || id == Tile::farmland->id) {
            constexpr int directions[][3] = {
                {0, BORDER, 0},
                {BORDER, 0, 0},
                {-BORDER, 0, 0},
                {0, 0, BORDER},
                {0, 0, -BORDER}
            };
            int out = 0;
            for (const int *dir : directions) {
                add_dir;
                if (is_in_chunk(0)) {
                    out = std::max(out, _get_raw_brightness(x, y, z, false));
                }
                sub_dir;
            }
            return out;
        }
    }
    return obj.raw_brightness;
}

// Implement LevelSource
int CachedLevelSource::getTile(const int x, const int y, const int z) {
    return block.id;
}
int CachedLevelSource::getData(const int x, const int y, const int z) {
    return block.data;
}
bool CachedLevelSource::isEmptyTile(const int x, const int y, const int z) {
    return block.is_empty;
}
float CachedLevelSource::getBrightness(const int x, const int y, const int z) {
    return block.brightness;
}
const Material *CachedLevelSource::getMaterial(const int x, const int y, const int z) {
    return block.material;
}
bool CachedLevelSource::_should_render(const int x, const int y, const int z) const {
    return block.should_render;
}