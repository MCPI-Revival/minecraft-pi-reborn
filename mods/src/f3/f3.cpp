#include <vector>
#include <iomanip>
#include <sstream>

#include <symbols/minecraft.h>
#include <libreborn/libreborn.h>

#include <mods/misc/misc.h>
#include <mods/feature/feature.h>
#include <mods/input/input.h>
#include <mods/version/version.h>
#include <mods/fps/fps.h>
#include <mods/init/init.h>

// Get Debug Information
static std::string to_string_with_precision(const double x, const int precision) {
    std::stringstream stream;
    stream << std::fixed << std::setprecision(precision) << x;
    return stream.str();
}
static int debug_precision = 3;
static std::vector<std::string> get_debug_info(const Minecraft *minecraft) {
    std::vector<std::string> info;
    // Version
    info.push_back(std::string("MCPI ") + version_get());
    // FPS
    info.push_back("FPS: " + to_string_with_precision(fps, debug_precision));
    // Seed
    if (minecraft->level) {
        info.push_back("");
        info.push_back("Seed: " + std::to_string(minecraft->level->data.seed));
    }
    // X/Y/Z
    if (minecraft->player) {
        info.push_back("");
        float x = minecraft->player->x;
        float y = minecraft->player->y - minecraft->player->height_offset;
        float z = minecraft->player->z;
        minecraft->command_server->pos_translator.to(x, y, z);
        info.push_back("X: " + to_string_with_precision(x, debug_precision));
        info.push_back("Y: " + to_string_with_precision(y, debug_precision));
        info.push_back("Z: " + to_string_with_precision(z, debug_precision));
    }
    // Return
    return info;
}

// Render Text With Background
static constexpr uint32_t debug_background_color = 0x90505050;
static constexpr int debug_text_color = 0xe0e0e0;
static void render_debug_line(Gui *gui, std::string &line, const int x, const int y) {
    // Draw Background
    const int width = gui->minecraft->font->width(line);
    if (width == 0) {
        return;
    }
    int x1 = x - 1;
    int y1 = y - 1;
    int x2 = x + width;
    int y2 = y + line_height;
    gui->fill(x1, y1, x2, y2, debug_background_color);
    // Draw Text
    gui->minecraft->font->draw(line, float(x), float(y), debug_text_color);
}
// Draw Debug Information
static bool debug_info_shown = false;
static constexpr int debug_margin = 2;
static constexpr int debug_line_padding = 1;
static void Gui_renderDebugInfo_injection(__attribute__((unused)) Gui_renderDebugInfo_t original, Gui *self) {
    if (debug_info_shown) {
        std::vector<std::string> info = get_debug_info(self->minecraft);
        int y = debug_margin;
        for (std::string &line : info) {
            render_debug_line(self, line, debug_margin, y);
            y += line_height;
            y += debug_line_padding;
        }
    }
}

// Init
void init_f3() {
    if (feature_has("F3 Debug Information", server_disabled)) {
        overwrite_calls(Gui_renderDebugInfo, Gui_renderDebugInfo_injection);
        misc_run_on_game_key_press([](__attribute__((unused)) Minecraft *minecraft, const int key) {
            if (key == MC_KEY_F3) {
                debug_info_shown = !debug_info_shown;
                return true;
            } else {
                return false;
            }
        });
    }
}