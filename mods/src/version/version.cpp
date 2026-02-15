#include <cstdint>

#include <libreborn/patch.h>
#include <libreborn/config.h>

#include <symbols/Strings.h>
#include <symbols/Common.h>
#include <symbols/GuiComponent.h>
#include <symbols/Font.h>

#include <mods/version/version.h>
#include <mods/init/init.h>

// Get New Version
const char *version_get() {
    static std::string version = "";
    // Load
    if (version.empty()) {
        version = std::string(Strings::minecraft_pi_version) + " / Reborn v" + reborn_config.general.version;
    }
    // Return
    return version.c_str();
}

// Injection For Touch GUI Version
static std::string Common_getGameVersionString_injection(MCPI_UNUSED Common_getGameVersionString_t original, MCPI_UNUSED const std::string &version_suffix) {
    // Set Version
    return version_get();
}

// Extra Text
static constexpr const char *extra_notice = "Unofficial Project / ";
static void StartMenuScreen_render_GuiComponent_drawString_injection(GuiComponent *self, Font *font, const std::string &text, int x, int y, const uint color) {
    // Add Notice
    x -= font->width(extra_notice);
    const std::string new_text = extra_notice + text;
    // Call Original Method
    self->drawString(font, new_text, x, y, color);
}

// Init
void init_version() {
    // Touch GUI
    overwrite_calls(Common_getGameVersionString, Common_getGameVersionString_injection);
    // Normal GUI
    patch_address((void *) &Strings::minecraft_pi_version, (void *) version_get());

    // Extra Text
    for (const uint32_t addr : {0x39750, 0x3e0b0}) {
        overwrite_call((void *) addr, GuiComponent_drawString, StartMenuScreen_render_GuiComponent_drawString_injection);
    }

    // Log
    INFO("Starting Minecraft: Pi Edition (%s)", version_get());
}
