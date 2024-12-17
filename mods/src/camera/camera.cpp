#include <libreborn/patch.h>

#include <symbols/minecraft.h>

#include <GLES/gl.h>

#include <mods/feature/feature.h>
#include <mods/screenshot/screenshot.h>
#include <mods/init/init.h>

// Take Screenshot Using TripodCamera
static void AppPlatform_saveScreenshot_injection(__attribute__((unused)) AppPlatform_saveScreenshot_t original, __attribute__((unused)) AppPlatform *app_platform, __attribute__((unused)) const std::string &path, __attribute__((unused)) int32_t width, __attribute__((unused)) int32_t height) {
    screenshot_take(nullptr);
}

// Enable TripodCameraRenderer
static EntityRenderDispatcher *EntityRenderDispatcher_injection(EntityRenderDispatcher_constructor_t original, EntityRenderDispatcher *dispatcher) {
    // Call Original Method
    original(dispatcher);

    // Register TripodCameraRenderer
    TripodCameraRenderer *renderer = TripodCameraRenderer::allocate();
    renderer->constructor();
    dispatcher->assign((unsigned char) 0x5, (EntityRenderer *) renderer);

    // Return
    return dispatcher;
}

// Display Smoke From TripodCamera Higher
static void TripodCamera_tick_Level_addParticle_call_injection(Level *level, const std::string &particle, float x, const float y, float z, float deltaX, float deltaY, float deltaZ, int count) {
    // Call Original Method
    level->addParticle(particle, x, y + 0.5, z, deltaX, deltaY, deltaZ, count);
}

// Fix Camera Legs
static void TripodCameraRenderer_render_EntityRenderer_bindTexture_injection(EntityRenderer *self, __attribute__((unused)) const std::string &file) {
    self->bindTexture("item/camera.png");
}
static void TripodCameraRenderer_render_TileRenderer_tesselateCrossTexture_injection(__attribute__((unused)) TileRenderer *self, __attribute__((unused)) Tile *tile, __attribute__((unused)) int data, __attribute__((unused)) float x, __attribute__((unused)) float y, __attribute__((unused)) float z) {
    Tesselator *t = &Tesselator::instance;
    for (const float a : {-1.f, 1.f}) {
        for (const float b : {-1.f, 1.f}) {
            constexpr float size = 0.45f;
            t->vertexUV(size * a,  0.5, size * b, 0.75, 0.5);
            t->vertexUV(size * a, -0.5, size * b, 0.75, 1);
            t->vertexUV(size * -a, -0.5, size * -b, 1, 1);
            t->vertexUV(size * -a,  0.5, size * -b, 1, 0.5);
        }
    }
}

// Init
void init_camera() {
    // Implement AppPlatform_linux::saveScreenshot So Cameras Work
    if (feature_has("Add Camera Functionality", server_disabled)) {
        overwrite_calls(AppPlatform_saveScreenshot, AppPlatform_saveScreenshot_injection);
    }

    // Fix Camera Rendering
    if (feature_has("Enable Camera Rendering", server_disabled)) {
        // Enable TripodCameraRenderer
        overwrite_calls(EntityRenderDispatcher_constructor, EntityRenderDispatcher_injection);
        // Display Smoke From TripodCamera Higher
        overwrite_call((void *) 0x87dc4, Level_addParticle, TripodCamera_tick_Level_addParticle_call_injection);
    }
    // Camera Legs
    if (feature_has("Render Camera Legs", server_disabled)) {
        overwrite_call((void *) 0x659dc, EntityRenderer_bindTexture, TripodCameraRenderer_render_EntityRenderer_bindTexture_injection);
        overwrite_call((void *) 0x65a08, TileRenderer_tesselateCrossTexture, TripodCameraRenderer_render_TileRenderer_tesselateCrossTexture_injection);
    }
}
