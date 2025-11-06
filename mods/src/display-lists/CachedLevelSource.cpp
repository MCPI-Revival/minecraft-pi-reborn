#include <bit>

#include "CachedLevelSource.h"

// Helpful Macros
#define block data[x - x0][z - z0][y - y0]
static constexpr unsigned int chunk_size = 16;
static constexpr unsigned int chunk_shift_factor = std::bit_width(chunk_size) - 1;
#define op_chunk_pos(x, op) ((x) op chunk_shift_factor)
#define to_chunk_pos(x) op_chunk_pos((x), >>)
#define from_chunk_pos(x) op_chunk_pos((x), <<)
#define to_local_pos(x) ((x) & (chunk_size - 1))
#define is_in_chunk(extra) \
    (x >= (x0 + (extra)) && x < (x1 - (extra)) \
    && y >= (y0 + (extra)) && y < (y1 - (extra)) && \
    z >= (z0 + (extra)) && z < (z1 - (extra)))
#define op_dir(op) x op dir[0]; y op dir[1]; z op dir[2]
#define add_dir op_dir(+=)
#define sub_dir op_dir(-=)

// Define cached region
// and copy tiles.
void CachedLevelSource::_cache(const int x0_, const int y0_, const int z0_, const int x1_, const int y1_, const int z1_) {
    x0 = x0_ - BORDER;
    y0 = y0_ - BORDER;
    z0 = z0_ - BORDER;
    x1 = x1_ + BORDER;
    y1 = y1_ + BORDER;
    z1 = z1_ + BORDER;
    _copy_tiles();
    _check_if_should_render();
}

// Copy Tile Information From Level
void CachedLevelSource::_copy_tiles() {
    // Raw Brightness Constants
    // See: Level::getRawBrightness
    constexpr int under_world_brightness = 0;
    constexpr int max_brightness = 15;
    const int over_world_brightness = std::max(max_brightness - level->field_6c, under_world_brightness);

    // Cache Tile Information
    for (int x = x0; x < x1; x++) {
        for (int z = z0; z < z1; z++) {
            // Get Chunk
            const int chunk_x = to_chunk_pos(x);
            const int min_local_x = std::max(x0, from_chunk_pos(chunk_x));
            const int chunk_z = to_chunk_pos(z);
            const int min_local_z = std::max(z0, from_chunk_pos(chunk_z));
            LevelChunk *&chunk = chunks[x - x0][z - z0];
            chunk = x == min_local_x && z == min_local_z
                ? level->getChunk(chunk_x, chunk_z)
                : chunks[min_local_x - x0][min_local_z - z0];

            // Copy Tiles
            for (int y = y0; y < y1; y++) {
                const bool over_world = y > MAX_Y;
                const bool valid_y = y >= MIN_Y && !over_world;
                Data &obj = block;
#define xyz to_local_pos(x), y, to_local_pos(z)
                obj.id = valid_y ? chunk->getTile(xyz) : 0;
                obj.data = valid_y ? chunk->getData(xyz) : 0;
                obj.raw_brightness = valid_y ? chunk->getRawBrightness(xyz, 0)
                    : (over_world ? over_world_brightness : under_world_brightness);
#undef xyz
                obj.is_empty = obj.id <= 0;
                obj.material = obj.is_empty ? Material::air : Tile::tiles[obj.id]->material;
                obj.should_render = false;
            }
        }
    }

    // Cache Brightness
    // This is a separate loop because brightness
    // calculations may require neighbor tiles.
    const float *light_ramp = level->dimension->light_ramp;
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