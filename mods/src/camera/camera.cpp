#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/feature/feature.h>
#include <mods/screenshot/screenshot.h>
#include <mods/home/home.h>
#include <mods/init/init.h>

// Take Screenshot Using TripodCamera
static void AppPlatform_linux_saveScreenshot_injection(__attribute__((unused)) AppPlatform *app_platform, __attribute__((unused)) std::string const& path, __attribute__((unused)) int32_t width, __attribute__((unused)) int32_t height) {
#ifndef MCPI_HEADLESS_MODE
    screenshot_take(home_get());
#endif
}

// Enable TripodCameraRenderer
static EntityRenderDispatcher *EntityRenderDispatcher_injection(EntityRenderDispatcher *dispatcher) {
    // Call Original Method
    (*EntityRenderDispatcher_constructor)(dispatcher);

    // Register TripodCameraRenderer
    TripodCameraRenderer *renderer = alloc_TripodCameraRenderer();
    ALLOC_CHECK(renderer);
    (*TripodCameraRenderer_constructor)(renderer);
    (*EntityRenderDispatcher_assign)(dispatcher, (unsigned char) 0x5, (EntityRenderer *) renderer);

    return dispatcher;
}

// Display Smoke From TripodCamera Higher
static void TripodCamera_tick_Level_addParticle_call_injection(Level *level, std::string *particle, float x, float y, float z, float deltaX, float deltaY, float deltaZ, int count) {
    // Call Original Method
    (*Level_addParticle)(level, particle, x, y + 0.5, z, deltaX, deltaY, deltaZ, count);
}

// Init
void init_camera() {
    // Implement AppPlatform_linux::saveScreenshot So Cameras Work
    patch_address(AppPlatform_linux_saveScreenshot_vtable_addr, (void *) AppPlatform_linux_saveScreenshot_injection);

    // Fix Camera Rendering
    if (feature_has("Fix Camera Rendering", server_disabled)) {
        // Enable TripodCameraRenderer
        overwrite_calls((void *) EntityRenderDispatcher_constructor, (void *) EntityRenderDispatcher_injection);
        // Display Smoke From TripodCamera Higher
        overwrite_call((void *) 0x87dc4, (void *) TripodCamera_tick_Level_addParticle_call_injection);
    }
}
