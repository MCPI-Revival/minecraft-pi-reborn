#include <GLES/gl.h>
#include <symbols/minecraft.h>
#include <libreborn/libreborn.h>

#include "shading-internal.h"

// OpenGL Lighting
static float *get_buffer(const float a, const float b, const float c, const float d) {
    static float buffer[4];
    buffer[0] = a;
    buffer[1] = b;
    buffer[2] = c;
    buffer[3] = d;
    return buffer;
}
static void lighting_turn_on() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    constexpr float a = 0.4f;
    constexpr float d = 0.6f;
    constexpr float s = 0.0f;
    Vec3 l = Vec3(0.2f, 1.0f, -0.7f).normalized();
    glLightfv(GL_LIGHT0, GL_POSITION, get_buffer(l.x, l.y, l.z, 0));
    glLightfv(GL_LIGHT0, GL_DIFFUSE, get_buffer(d, d, d, 1));
    glLightfv(GL_LIGHT0, GL_AMBIENT, get_buffer(0, 0, 0, 1));
    glLightfv(GL_LIGHT0, GL_SPECULAR, get_buffer(s, s, s, 1.0f));
    l = Vec3(-0.2f, 1.0f, 0.7f).normalized();
    glLightfv(GL_LIGHT1, GL_POSITION, get_buffer(l.x, l.y, l.z, 0));
    glLightfv(GL_LIGHT1, GL_DIFFUSE, get_buffer(d, d, d, 1));
    glLightfv(GL_LIGHT1, GL_AMBIENT, get_buffer(0, 0, 0, 1));
    glLightfv(GL_LIGHT1, GL_SPECULAR, get_buffer(s, s, s, 1.0f));
    glShadeModel(GL_FLAT);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, get_buffer(a, a, a, 1));
}
static void lighting_turn_off() {
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_COLOR_MATERIAL);
}

// Entity Rendering
static void LevelRenderer_renderEntities_injection(LevelRenderer_renderEntities_t original, LevelRenderer *self, Vec3 pos, void *unknown, float a) {
    lighting_turn_on();
    original(self, pos, unknown, a);
    lighting_turn_off();
}

// Held Items
static void ItemInHandRenderer_render_glPopMatrix_injection() {
    lighting_turn_on();
    glPopMatrix();
    glEnable(GL_RESCALE_NORMAL);
}
static void ItemInHandRenderer_render_injection(ItemInHandRenderer_render_t original, ItemInHandRenderer *self, float a) {
    original(self, a);
    lighting_turn_off();
    glDisable(GL_RESCALE_NORMAL);
}

// GL_RESCALE_NORMAL
static void enable_rescale_normal() {
    glPushMatrix();
    glEnable(GL_RESCALE_NORMAL);
}
static void disable_rescale_normal() {
    glPopMatrix();
    glDisable(GL_RESCALE_NORMAL);
}
template <typename Self>
static void EntityRenderer_render_injection(const std::function<void(Self *, Entity *, float, float, float, float, float)> &original, Self *self, Entity *entity, float x, float y, float z, float rot, float a) {
    glEnable(GL_RESCALE_NORMAL);
    original(self, entity, x, y, z, rot, a);
    glDisable(GL_RESCALE_NORMAL);
}

// Fix Falling Tile Rendering
static void FallingTileRenderer_render_TileRenderer_renderBlock_injection(TileRenderer *self, Tile *tile, LevelSource *level, int x, int y, int z) {
    glDisable(GL_LIGHTING);
    self->renderBlock(tile, level, x, y, z);
    glEnable(GL_LIGHTING);
}
static void TntRenderer_render_TileRenderer_renderTile_injection(TileRenderer *self, Tile *tile, int data) {
    glDisable(GL_LIGHTING);
    self->renderTile(tile, data);
    glEnable(GL_LIGHTING);
}

// Fix Names
static void MobRenderer_renderNameTag_injection(MobRenderer_renderNameTag_t original, MobRenderer *self, Mob *mob, const std::string &name, const float x, const float y, const float z, const int param_1) {
    glDisable(GL_LIGHTING);
    original(self, mob, name, x, y, z, param_1);
    glEnable(GL_LIGHTING);
}

// Armor Screen
static void ArmorScreen_renderPlayer_injection(ArmorScreen_renderPlayer_t original, ArmorScreen *self, float param_1, float param_2) {
    lighting_turn_on();
    original(self, param_1, param_2);
    lighting_turn_off();
}

// Init
void _init_lighting() {
    overwrite_calls(LevelRenderer_renderEntities, LevelRenderer_renderEntities_injection);
    overwrite_call((void *) 0x4c04c, (void *) ItemInHandRenderer_render_glPopMatrix_injection);
    overwrite_calls(ItemInHandRenderer_render, ItemInHandRenderer_render_injection);
    overwrite_call((void *) 0x4bedc, (void *) enable_rescale_normal);
    overwrite_call((void *) 0x4bf70, (void *) disable_rescale_normal);
    overwrite_calls(ItemRenderer_render, EntityRenderer_render_injection<ItemRenderer>);
    overwrite_calls(ArrowRenderer_render, EntityRenderer_render_injection<ArrowRenderer>);
    overwrite_calls(ItemSpriteRenderer_render, EntityRenderer_render_injection<ItemSpriteRenderer>);
    overwrite_calls(PaintingRenderer_render, EntityRenderer_render_injection<PaintingRenderer>);
    overwrite_call((void *) 0x641ec, (void *) enable_rescale_normal);
    overwrite_call((void *) 0x647a0, (void *) disable_rescale_normal);
    overwrite_call((void *) 0x62b08, (void *) FallingTileRenderer_render_TileRenderer_renderBlock_injection);
    overwrite_call((void *) 0x65754, (void *) TntRenderer_render_TileRenderer_renderTile_injection);
    overwrite_calls(MobRenderer_renderNameTag, MobRenderer_renderNameTag_injection);
    overwrite_calls(ArmorScreen_renderPlayer, ArmorScreen_renderPlayer_injection);
}