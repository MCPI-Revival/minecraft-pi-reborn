#include <libreborn/patch.h>
#include <GLES/gl.h>

#include <symbols/Tesselator.h>
#include <symbols/EntityRenderDispatcher.h>
#include <symbols/Entity.h>
#include <symbols/LevelRenderer.h>
#include <symbols/Chunk.h>
#include <symbols/Minecraft.h>
#include <symbols/Mob.h>

#include <mods/misc/misc.h>
#include <mods/feature/feature.h>
#include <mods/common.h>

#include "internal.h"

// Box Drawing
static void draw_box(const float x0, const float y0, const float z0, const float x1, const float y1, const float z1) {
    Tesselator &t = Tesselator::instance;
    const float vertices[][3] = {
        {x0, y0, z0}, {x1, y0, z0}, {x1, y1, z0}, {x0, y1, z0},
        {x0, y0, z1}, {x1, y0, z1}, {x1, y1, z1}, {x0, y1, z1}
    };
    const int edges[][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };
    for (const int (&e)[2] : edges) {
        const float (&a)[3] = vertices[e[0]];
        const float (&b)[3] = vertices[e[1]];
        t.vertex(a[0], a[1], a[2]);
        t.vertex(b[0], b[1], b[2]);
    }
}

// Line Drawing
static void begin_lines() {
    misc_set_nice_line_width();
    media_glColor4f(1, 1, 1, 1);
    Tesselator &t = Tesselator::instance;
    t.begin(GL_LINES);
}
static void end_lines(const bool disable_lighting) {
    std::vector<GLenum> flags;
    flags.push_back(GL_TEXTURE_2D);
    flags.push_back(GL_FOG);
    if (disable_lighting) {
        flags.push_back(GL_LIGHTING);
    }
    for (const GLenum flag : flags) {
        media_glDisable(flag);
    }
    Tesselator &t = Tesselator::instance;
    t.draw();
    for (const GLenum flag : flags) {
        media_glEnable(flag);
    }
}

// Entity Hitboxes
static void EntityRenderDispatcher_render_injection(EntityRenderDispatcher_render_t original, EntityRenderDispatcher *self, Entity *entity, const float x, const float y, const float z, const float rot, const float unknown) {
    // Call Original Method
    original(self, entity, x, y, z, rot, unknown);
    // Draw Box
    begin_lines();
    const float half_width = entity->hitbox_width / 2;
    const float height = entity->hitbox_height;
    const float real_y = y - entity->height_offset;
    draw_box(x - half_width, real_y, z - half_width, x + half_width, real_y + height, z + half_width);
    end_lines(true);
}

// Box Drawing With A Grid
static constexpr int grid_size = 2;
static void draw_box_with_grid(const int x0, const int y0, const int z0, const int x1, const int y1, const int z1) {
    draw_box(float(x0), float(y0), float(z0), float(x1), float(y1), float(z1));
    void (*line)(int, int, int, int, int, int) = [](const int ax, const int ay, const int az, const int bx, const int by, const int bz) {
        Tesselator &t = Tesselator::instance;
        t.vertex(float(ax), float(ay), float(az));
        t.vertex(float(bx), float(by), float(bz));
    };
    for (int x = x0 + grid_size; x < x1; x += grid_size) {
        line(x, y0, z0, x, y1, z0);
        line(x, y0, z1, x, y1, z1);
        line(x, y0, z0, x, y0, z1);
        line(x, y1, z0, x, y1, z1);
    }
    for (int y = y0 + grid_size; y < y1; y += grid_size) {
        line(x0, y, z0, x1, y, z0);
        line(x0, y, z1, x1, y, z1);
        line(x0, y, z0, x0, y, z1);
        line(x1, y, z0, x1, y, z1);
    }
    for (int z = z0 + grid_size; z < z1; z += grid_size) {
        line(x0, y0, z, x1, y0, z);
        line(x0, y1, z, x1, y1, z);
        line(x0, y0, z, x0, y1, z);
        line(x1, y0, z, x1, y1, z);
    }
}

// Chunks
static bool outline_render_chunks;
static bool outline_level_chunks;
static int LevelRenderer_renderChunks_injection(LevelRenderer_renderChunks_t original, LevelRenderer *self, const int start, const int end, const int a, const float b) {
    // Call Original Method
    const int ret = original(self, start, end, a, b);
    if (a != 0) {
        return ret;
    }

    // Start Drawing
    begin_lines();
    Mob *camera = self->minecraft->camera;
    const float offset_x = camera->old_x + ((camera->x - camera->old_x) * b);
    const float offset_y = camera->old_y + ((camera->y - camera->old_y) * b);
    const float offset_z = camera->old_z + ((camera->z - camera->old_z) * b);
    Tesselator &t = Tesselator::instance;
    t.addOffset(-offset_x, -offset_y, -offset_z);

    // Render Chunks
    if (outline_render_chunks) {
        // Search For Chunk Containing The Camera
        for (int i = start; i < end; i++) {
            const Chunk *chunk = self->chunks[i];
            if (chunk) {
                const int x0 = chunk->x;
                const int x1 = x0 + chunk->width;
                const int y0 = chunk->y;
                const int y1 = y0 + chunk->height;
                const int z0 = chunk->z;
                const int z1 = z0 + chunk->depth;
                // Check Whether Camera Is Inside
                const bool inside_x = camera->x >= float(x0) && camera->x < float(x1);
                const float camera_y = camera->y + camera->getHeadHeight();
                const bool inside_y = camera_y >= float(y0) && camera_y < float(y1);
                const bool inside_z = camera->z >= float(z0) && camera->z < float(z1);
                if (inside_x && inside_y && inside_z) {
                    // Found The Chunk
                    draw_box_with_grid(x0, y0, z0, x1, y1, z1);
                    break;
                }
            }
        }
    }

    // Level Chunk
    if (outline_level_chunks) {
        // Constants
        constexpr int level_chunk_size = LevelSize::CHUNK_SIZE;
        constexpr int level_chunk_height = LevelSize::HEIGHT;
        // Get Chunk Containing The Player
        const int chunk_x = int(camera->x) / level_chunk_size;
        const int chunk_z = int(camera->z) / level_chunk_size;
        // Draw The Box
        const int x0 = chunk_x * level_chunk_size;
        const int x1 = x0 + level_chunk_size;
        constexpr int y0 = 0;
        constexpr int y1 = level_chunk_height;
        const int z0 = chunk_z * level_chunk_size;
        const int z1 = z0 + level_chunk_size;
        draw_box_with_grid(x0, y0, z0, x1, y1, z1);
    }

    // Return
    t.addOffset(offset_x, offset_y, offset_z);
    end_lines(false);
    return ret;
}

// Init
void _init_f3_outlining() {
    if (feature_has("Outline Entity Hitboxes", server_disabled)) {
        overwrite_calls(EntityRenderDispatcher_render, EntityRenderDispatcher_render_injection);
    }
    outline_render_chunks = feature_has("Outline Render Chunks", server_disabled);
    outline_level_chunks = feature_has("Outline Level Chunks", server_disabled);
    if (outline_render_chunks || outline_level_chunks) {
        overwrite_calls(LevelRenderer_renderChunks, LevelRenderer_renderChunks_injection);
    }
}