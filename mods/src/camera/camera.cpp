#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/feature/feature.h>
#include <mods/screenshot/screenshot.h>
#include <mods/home/home.h>
#include <mods/init/init.h>

// Take Screenshot Using TripodCamera
static void AppPlatform_saveScreenshot_injection(__attribute__((unused)) AppPlatform_saveScreenshot_t original, __attribute__((unused)) AppPlatform *app_platform, __attribute__((unused)) std::string *path, __attribute__((unused)) int32_t width, __attribute__((unused)) int32_t height) {
#ifndef MCPI_HEADLESS_MODE
    screenshot_take(home_get());
#endif
}

// Enable TripodCameraRenderer
static EntityRenderDispatcher *EntityRenderDispatcher_injection(EntityRenderDispatcher_constructor_t original, EntityRenderDispatcher *dispatcher) {
    // Call Original Method
    original(dispatcher);

    // Register TripodCameraRenderer
    TripodCameraRenderer *renderer = new TripodCameraRenderer;
    ALLOC_CHECK(renderer);
    renderer->constructor();
    dispatcher->assign((unsigned char) 0x5, (EntityRenderer *) renderer);

    return dispatcher;
}

// Display Smoke From TripodCamera Higher
static void TripodCamera_tick_Level_addParticle_call_injection(Level *level, std::string *particle, float x, float y, float z, float deltaX, float deltaY, float deltaZ, int count) {
    // Call Original Method
    level->addParticle(particle, x, y + 0.5, z, deltaX, deltaY, deltaZ, count);
}

// Init
void init_camera() {
    // Implement AppPlatform_linux::saveScreenshot So Cameras Work
    overwrite_calls(AppPlatform_saveScreenshot, AppPlatform_saveScreenshot_injection);

    // Fix Camera Rendering
    if (feature_has("Fix Camera Rendering", server_disabled)) {
        // Enable TripodCameraRenderer
        overwrite_calls(EntityRenderDispatcher_constructor, EntityRenderDispatcher_injection);
        // Display Smoke From TripodCamera Higher
        overwrite_call((void *) 0x87dc4, (void *) TripodCamera_tick_Level_addParticle_call_injection);
    }
}
