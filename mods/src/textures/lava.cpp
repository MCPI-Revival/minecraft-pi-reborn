#include <cmath>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "textures-internal.h"

// Lava texture code was originally decompield by @iProgramMC as part of ReMinecraftPE.
// See: https://github.com/ReMinecraftPE/mcpe

// Structures
struct LavaTexture {
    DynamicTexture super;
    int field_14;
    int field_18;
    float m_data1[256];
    float m_data2[256];
    float m_data3[256];
    float m_data4[256];
};
struct LavaSideTexture {
    DynamicTexture super;
    int field_14;
    int field_18;
    int field_1C;
    float m_data1[256];
    float m_data2[256];
    float m_data3[256];
    float m_data4[256];
};
struct FireTexture {
    DynamicTexture super;
    float m_data1[320];
    float m_data2[320];
    Random m_random;
};

// LavaTexture
CUSTOM_VTABLE(lava_texture, DynamicTexture) {
    vtable->tick = [](DynamicTexture *super) {
        LavaTexture *self = (LavaTexture *) super;
        for (int x = 0; x < 16; x++) {
            for (int y = 0; y < 16; y++) {
                float f = 0.0F;
                int ax = int(Mth_sin((float(x) * float(M_PI) * 2) / 16.0f) * 1.2f);
                int ay = int(Mth_sin((float(y) * float(M_PI) * 2) / 16.0f) * 1.2f);
                for (int bx = x - 1; bx <= x + 1; bx++) {
                    for (int by = y - 1; by <= y + 1; by++) {
                        int k2 = (bx + ay) & 0xf;
                        int i3 = (by + ax) & 0xf;
                        f += self->m_data1[k2 + i3 * 16];
                    }
                }
                self->m_data2[x + y * 16] = f / 10.0f + ((self->m_data3[(x & 0xf) + ((y + 0) & 0xf) * 16] + self->m_data3[((x + 1) & 0xf) + (y & 0xf) * 16] + self->m_data3[((x + 1) & 0xf) + ((y + 1) & 0xf) * 16] + self->m_data3[(x & 0xf) + ((y + 1) & 0xf) * 16]) * 0.25f) * 0.8f;
                self->m_data3[x + y * 16] += self->m_data4[x + y * 16] * 0.01f;
                if (self->m_data3[x + y * 16] < 0.0f) {
                    self->m_data3[x + y * 16] = 0.0f;
                }
                self->m_data4[x + y * 16] -= 0.06f;
                if (Mth_random() < 0.005f) {
                    self->m_data4[x + y * 16] = 1.5f;
                }
            }
        }
        std::swap(self->m_data1, self->m_data2);
        for (int i = 0; i < 256; i++) {
            float x1 = self->m_data1[i] * 2.0f;
            if (x1 > 1.0f) {
                x1 = 1.0f;
            }
            if (x1 < 0.0f) {
                x1 = 0.0f;
            }
            self->super.pixels[i * 4 + 0] = int(155.0f + 100.0f * x1);
            self->super.pixels[i * 4 + 1] = int(255.0f * x1 * x1);
            self->super.pixels[i * 4 + 2] = int(128.0f * x1 * x1 * x1 * x1);
            self->super.pixels[i * 4 + 3] = 255;
        }
    };
}
static DynamicTexture *create_lava_texture() {
    // Construct
    LavaTexture *texture = new LavaTexture;
    ALLOC_CHECK(texture);
    DynamicTexture_constructor(&texture->super, Tile_lava->texture);
    // Set VTable
    texture->super.vtable = get_lava_texture_vtable();
    // Setup
    texture->field_14 = 0;
    texture->field_18 = 0;
    for (int i = 0; i < 256; i++) {
        texture->m_data1[i] = 0.0f;
        texture->m_data2[i] = 0.0f;
        texture->m_data3[i] = 0.0f;
        texture->m_data4[i] = 0.0f;
    }
    // Return
    return (DynamicTexture *) texture;
}

// LavaSideTexture
CUSTOM_VTABLE(lava_side_texture, DynamicTexture) {
    vtable->tick = [](DynamicTexture *super) {
        LavaSideTexture *self = (LavaSideTexture *) super;
        self->field_1C++;
        for (int x = 0; x < 16; x++) {
            for (int y = 0; y < 16; y++) {
                float f = 0.0F;
                int ax = int(Mth_sin((float(x) * float(M_PI) * 2) / 16.0f) * 1.2f);
                int ay = int(Mth_sin((float(y) * float(M_PI) * 2) / 16.0f) * 1.2f);
                for (int bx = x - 1; bx <= x + 1; bx++) {
                    for (int by = y - 1; by <= y + 1; by++) {
                        int k2 = (bx + ay) & 0xf;
                        int i3 = (by + ax) & 0xf;
                        f += self->m_data1[k2 + i3 * 16];
                    }
                }
                self->m_data2[x + y * 16] = f / 10.0f + ((self->m_data3[(x & 0xf) + ((y + 0) & 0xf) * 16] + self->m_data3[((x + 1) & 0xf) + (y & 0xf) * 16] + self->m_data3[((x + 1) & 0xf) + ((y + 1) & 0xf) * 16] + self->m_data3[(x & 0xf) + ((y + 1) & 0xf) * 16]) * 0.25f) * 0.8f;
                self->m_data3[x + y * 16] += self->m_data4[x + y * 16] * 0.01f;
                if (self->m_data3[x + y * 16] < 0.0f) {
                    self->m_data3[x + y * 16] = 0.0f;
                }
                self->m_data4[x + y * 16] -= 0.06f;
                if (Mth_random() < 0.005f) {
                    self->m_data4[x + y * 16] = 1.5f;
                }
            }
        }
        std::swap(self->m_data1, self->m_data2);
        for (int i = 0; i < 256; i++) {
            float x1 = self->m_data1[(i - 16 * (self->field_1C / 3)) & 0xFF] * 2.0f;
            if (x1 > 1.0f) {
                x1 = 1.0f;
            }
            if (x1 < 0.0f) {
                x1 = 0.0f;
            }
            self->super.pixels[i * 4 + 0] = int(155.0f + 100.0f * x1);
            self->super.pixels[i * 4 + 1] = int(255.0f * x1 * x1);
            self->super.pixels[i * 4 + 2] = int(128.0f * x1 * x1 * x1 * x1);
            self->super.pixels[i * 4 + 3] = 255;
        }
    };
}
static DynamicTexture *create_lava_side_texture() {
    // Construct
    LavaSideTexture *texture = new LavaSideTexture;
    ALLOC_CHECK(texture);
    DynamicTexture_constructor(&texture->super, Tile_lava->texture + 1);
    // Set VTable
    texture->super.vtable = get_lava_side_texture_vtable();
    // Setup
    texture->field_14 = 0;
    texture->field_18 = 0;
    texture->field_1C = 0;
    texture->super.texture_size = 2;
    for (int i = 0; i < 256; i++) {
        texture->m_data1[i] = 0.0f;
        texture->m_data2[i] = 0.0f;
        texture->m_data3[i] = 0.0f;
        texture->m_data4[i] = 0.0f;
    }
    // Return
    return (DynamicTexture *) texture;
}

// FireTexture
CUSTOM_VTABLE(fire_texture, DynamicTexture) {
    vtable->tick = [](DynamicTexture *super) {
        FireTexture *self = (FireTexture *) super;
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 20; j++) {
                int l = 18;
                float f1 = self->m_data1[i + ((j + 1) % 20) * 16] * l;
                for (int i1 = i - 1; i1 <= i + 1; i1++) {
                    for (int k1 = j; k1 <= j + 1; k1++) {
                        int i2 = i1;
                        int k2 = k1;
                        if (i2 >= 0 && k2 >= 0 && i2 < 16 && k2 < 20)
                        {
                            f1 += self->m_data1[i2 + k2 * 16];
                        }
                        l++;
                    }
                }
                self->m_data2[i + j * 16] = f1 / 25.2f;
                if (j >= 19) {
                    union {
                        uint32_t x;
                        uint8_t b[4];
                    } a;
                    a.x = Random_genrand_int32(&self->m_random);
                    self->m_data2[i + j * 16] = 0.2f + (((a.b[3] / 256.0f) * 0.1f) + ((((a.b[0] / 256.0f) * (a.b[1] / 256.0f)) * (a.b[2] / 256.0f)) * 4.0f));
                }
            }
        }
        std::swap(self->m_data1, self->m_data2);
        for (int i = 0; i < 256; i++) {
            float x = self->m_data1[i] * 1.8f;
            if (x > 1.0f) {
                x = 1.0f;
            }
            if (x < 0.0f) {
                x = 0.0f;
            }
            self->super.pixels[4 * i + 0] = int(x * 155.0f + 100.0f);
            self->super.pixels[4 * i + 1] = int(x * x * 255.0f);
            self->super.pixels[4 * i + 2] = int(x * x * x * x * x * x * x * x * x * x * 255.0f);
            self->super.pixels[4 * i + 3] = x >= 0.5f ? 255 : 0;
        }
    };
}
static DynamicTexture *create_fire_texture(int a2) {
    // Construct
    FireTexture *texture = new FireTexture;
    ALLOC_CHECK(texture);
    DynamicTexture_constructor(&texture->super, Tile_fire->texture + (16 * a2));
    // Set VTable
    texture->super.vtable = get_fire_texture_vtable();
    // Setup Random
    int seed = Common_getTimeMs();
    texture->m_random.seed = seed;
    texture->m_random.param_1 = 0x271;
    texture->m_random.param_2 = false;
    texture->m_random.param_3 = 0;
    for (int i = 0; i < 320; i++) {
        texture->m_data1[i] = 0.0f;
        texture->m_data2[i] = 0.0f;
    }
    // Return
    return (DynamicTexture *) texture;
}

// Add Textures
static bool animated_water = false;
static bool animated_lava = false;
static bool animated_fire = false;
static void Textures_addDynamicTexture_injection(Textures *textures, DynamicTexture *dynamic_texture) {
    // Call Original Method
    if (animated_water) {
        Textures_addDynamicTexture(textures, dynamic_texture);
    }

    // Add Lava
    if (animated_lava) {
        Textures_addDynamicTexture(textures, create_lava_texture());
        Textures_addDynamicTexture(textures, create_lava_side_texture());
    }
    if (animated_fire) {
        Textures_addDynamicTexture(textures, create_fire_texture(0));
        Textures_addDynamicTexture(textures, create_fire_texture(1));
    }
}

// Init
void _init_textures_lava(bool animated_water_param, bool animated_lava_param, bool animated_fire_param) {
    animated_water = animated_water_param;
    animated_lava = animated_lava_param;
    animated_fire = animated_fire_param;
    overwrite_call((void *) 0x170b4, (void *) Textures_addDynamicTexture_injection);
}
