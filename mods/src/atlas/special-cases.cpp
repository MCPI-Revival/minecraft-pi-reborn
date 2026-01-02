#include <libreborn/patch.h>
#include <GLES/gl.h>

#include <symbols/CropTile.h>
#include <symbols/Tesselator.h>
#include <symbols/GuiComponent.h>
#include <symbols/Textures.h>
#include <symbols/Gui.h>

#include "internal.h"

// Fix Buggy Crop Textures
thread_local bool _atlas_active = false;
#define MAX_CROP_DATA 7
static int CropTile_getTexture2_injection(CropTile_getTexture2_t original, CropTile *self, const int face, int data) {
    if (data > MAX_CROP_DATA && _atlas_active) {
        data = MAX_CROP_DATA;
    }
    return original(self, face, data);
}

// Fix Open Inventory Button
void _atlas_copy_inventory_button(Textures *textures, Gui *gui) {
    textures->loadAndBindTexture("terrain.png");
    constexpr int in_icon_x = 209;
    constexpr int in_icon_y = 214;
    constexpr int in_icon_width = 14;
    constexpr int in_icon_height = 4;
    constexpr int out_icon_width = in_icon_width * 2;
    constexpr int out_icon_height = in_icon_height * 2;
    constexpr int out_icon_x = atlas_texture_size - out_icon_width;
    constexpr int out_icon_y = atlas_texture_size - out_icon_height;
    gui->blit(out_icon_x, out_icon_y, in_icon_x, in_icon_y, out_icon_width, out_icon_height, in_icon_width, in_icon_height);
}
static void Gui_renderToolBar_GuiComponent_blit_injection(GuiComponent *self, const int x_dest, const int y_dest, MCPI_UNUSED const int x_src, MCPI_UNUSED const int y_src, const int width_dest, const int height_dest, const int width_src, const int height_src) {
    constexpr float size_scale = 2.0f / atlas_texture_size;
    constexpr float u1 = 1.0f;
    const float u0 = u1 - (float(width_src) * size_scale);
    constexpr float v1 = 1.0f;
    const float v0 = v1 - (float(height_src) * size_scale);
    Tesselator &t = Tesselator::instance;
    t.begin(GL_QUADS);
    t.vertexUV(float(x_dest), float(y_dest + height_dest), self->z, u0, v1);
    t.vertexUV(float(x_dest + width_dest), float(y_dest + height_dest), self->z, u1, v1);
    t.vertexUV(float(x_dest + width_dest), float(y_dest), self->z, u1, v0);
    t.vertexUV(float(x_dest), float(y_dest), self->z, u0, v0);
    t.draw();
}

// Init
void _atlas_init_special_cases() {
    overwrite_calls(CropTile_getTexture2, CropTile_getTexture2_injection);
    overwrite_call((void *) 0x26f50, GuiComponent_blit, Gui_renderToolBar_GuiComponent_blit_injection);
}