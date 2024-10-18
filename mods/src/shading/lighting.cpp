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

// Init
void _init_lighting() {
    overwrite_calls(LevelRenderer_renderEntities, LevelRenderer_renderEntities_injection);
}