#include <GLES/gl.h>

#include <mods/shading/FallingSandRenderer.h>
#include "FallingSandRenderer.h"

// Custom Level Source
int FallingSandRenderer::SandLevelSource::getTile(const int x, const int y, const int z) {
    if (x == real_x && y == real_y && z == real_z) {
        return id;
    } else {
        return 0;
    }
}
int FallingSandRenderer::SandLevelSource::getData(const int x, const int y, const int z) {
    if (x == real_x && y == real_y && z == real_z) {
        return data;
    } else {
        return 0;
    }
}
const Material *FallingSandRenderer::SandLevelSource::getMaterial(const int x, const int y, const int z) {
    if (x == real_x && y == real_y && z == real_z && id > 0) {
        return Tile::tiles[id]->material;
    } else {
        return Material::air;
    }
}
float FallingSandRenderer::SandLevelSource::getBrightness(MCPI_UNUSED const int x, MCPI_UNUSED const int y, MCPI_UNUSED const int z) {
    return brightness;
}

// Constructor
FallingSandRenderer *get_falling_sand_renderer() {
    static FallingSandRenderer renderer;
    return &renderer;
}
FallingSandRenderer::FallingSandRenderer() {
    // Construct Block Renderer
    renderer = TileRenderer::allocate();
    renderer->constructor(level_source.self);
    renderer->disable_culling = true;
}

// Render
bool FallingSandRenderer::render() const {
    // Render Block
    Tile *tile = Tile::tiles[level_source.id];
    if (tile) {
        // Offset Position
        media_glPushMatrix();
        constexpr float offset = 0.5f;
        const int x = level_source.real_x;
        const float offset_x = offset + float(x);
        const int y = level_source.real_y;
        const float offset_y = offset + float(y);
        const int z = level_source.real_z;
        const float offset_z = offset + float(z);
        media_glTranslatef(-offset_x, -offset_y, -offset_z);
        // Render
        Tesselator &t = Tesselator::instance;
        t.begin(GL_QUADS);
        const bool success = renderer->tesselateInWorld(tile, x, y, z);
        t.draw();
        media_glPopMatrix();
        return success;
    } else {
        // Nothing To Render
        // Treat This As Success
        return true;
    }
}

// Retrieve Level
Level *get_level_from_falling_sand_renderer(const LevelSource *level_source) {
    const FallingSandRenderer::SandLevelSource &sand_level_source = get_falling_sand_renderer()->level_source;
    if (level_source == sand_level_source.self) {
        return sand_level_source.level;
    } else {
        return nullptr;
    }
}