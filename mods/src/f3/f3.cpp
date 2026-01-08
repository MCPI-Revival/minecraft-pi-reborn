#include <vector>
#include <iomanip>
#include <sstream>
#include <cmath>

#include <symbols/Minecraft.h>
#include <symbols/Level.h>
#include <symbols/ClientSideNetworkHandler.h>
#include <symbols/LocalPlayer.h>
#include <symbols/CommandServer.h>
#include <symbols/Tile.h>
#include <symbols/I18n.h>
#include <symbols/Entity.h>
#include <symbols/Font.h>
#include <symbols/Tesselator.h>

#include <libreborn/patch.h>
#include <libreborn/util/string.h>
#include <GLES/gl.h>

#include <mods/misc/misc.h>
#include <mods/feature/feature.h>
#include <mods/input/input.h>
#include <mods/version/version.h>
#include <mods/fps/fps.h>
#include <mods/init/init.h>

#include "internal.h"

// Get Debug Information
static std::string to_string_with_precision(const double x, const int precision) {
    std::stringstream stream;
    stream << std::fixed << std::setprecision(precision) << x;
    return stream.str();
}
static float wrap_degrees(float x) {
    x = std::fmod(x, 360);
    if (x >= 180) {
        x -= 360;
    }
    if (x < -180) {
        x += 360;
    }
    return x;
}
static int debug_precision = 3;
static std::vector<std::string> get_debug_info_left(const Minecraft *minecraft) {
    std::vector<std::string> info;
    // Version
    info.push_back(std::string("MCPI ") + version_get());
    // FPS
    info.push_back("FPS: " + to_string_with_precision(fps, debug_precision));
    // Level Information
    if (minecraft->level) {
        info.push_back("");
        info.push_back("Seed: " + safe_to_string(minecraft->level->data.seed));
        const ClientSideNetworkHandler *handler = (ClientSideNetworkHandler *) minecraft->network_handler;
        if (handler && handler->vtable == ClientSideNetworkHandler::VTable::base) {
            int total_chunks = 0;
            int loaded_chunks = 0;
            for (const bool &chunk_loaded : handler->chunk_loaded) {
                total_chunks++;
                if (chunk_loaded) {
                    loaded_chunks++;
                }
            }
            info.push_back("Chunks Loaded: " + safe_to_string(loaded_chunks) + '/' + safe_to_string(total_chunks));
        }
        info.push_back("Time: " + safe_to_string(minecraft->level->data.time));
        info.push_back("Entities: " + safe_to_string(minecraft->level->entities.size()));
        info.push_back("Players: " + safe_to_string(minecraft->level->players.size()));
    }
    // Player Information
    if (minecraft->player) {
        // X/Y/Z
        info.push_back("");
        float x = minecraft->player->x;
        float y = minecraft->player->y - minecraft->player->height_offset;
        float z = minecraft->player->z;
        minecraft->command_server->pos_translator.to_float(x, y, z);
        info.push_back("X: " + to_string_with_precision(x, debug_precision));
        info.push_back("Y: " + to_string_with_precision(y, debug_precision));
        info.push_back("Z: " + to_string_with_precision(z, debug_precision));
        // Rotation
        info.push_back("");
        const float yaw = wrap_degrees(minecraft->player->yaw);
        info.push_back("Yaw: " + to_string_with_precision(yaw, debug_precision));
        const float pitch = wrap_degrees(minecraft->player->pitch);
        info.push_back("Pitch: " + to_string_with_precision(pitch, debug_precision));
        // Facing
        char axis[3] = {};
        bool is_positive_on_axis;
        std::string direction;
        if (const float abs_yaw = std::abs(yaw); abs_yaw < 45 || abs_yaw > 135) {
            // Z-Axis
            axis[1] = 'Z';
            is_positive_on_axis = abs_yaw < 90;
            direction = is_positive_on_axis ? "South" : "North";
        } else {
            // X-Axis
            axis[1] = 'X';
            is_positive_on_axis = yaw < 0;
            direction = is_positive_on_axis ? "East" : "West";
        }
        axis[0] = is_positive_on_axis ? '+' : '-';
        info.push_back("Facing: " + direction + " (" + axis + ")");
    }
    // Return
    return info;
}
static std::string format_type(const int id, const std::string &name) {
    std::string out = safe_to_string(id);
    if (!name.empty()) {
        out = name + " (" + out + ')';
    }
    out = "Type: " + out;
    return out;
}
static std::vector<std::string> get_debug_info_right(const Minecraft *minecraft) {
    std::vector<std::string> info;
    // TPS
    info.push_back("TPS: " + to_string_with_precision(tps, debug_precision));
    // Chunk Updates
    info.push_back("Chunk Updates: " + to_string_with_precision(chunk_updates, debug_precision));
    // Target Information
    const HitResult &target = minecraft->hit_result;
    if (target.type != 2) {
        float x;
        float y;
        float z;
        std::string type;
        std::vector<std::string> type_info;
        int xyz_precision;
        if (target.type == 0) {
            // Tile
            x = float(target.x);
            y = float(target.y);
            z = float(target.z);
            type = "Tile";
            if (minecraft->level) {
                const int id = minecraft->level->getTile(int(x), int(y), int(z));
                std::string name;
                if (const Tile *tile = Tile::tiles[id]) {
                    const std::string description_id = tile->getDescriptionId();
                    name = description_id + ".name";
                    if (I18n::_strings.contains(name)) {
                        name = I18n::_strings[name];
                    } else {
                        name = description_id;
                    }
                }
                type_info.push_back(format_type(id, name));
                type_info.push_back("Data: " + safe_to_string(minecraft->level->getData(int(x), int(y), int(z))));
            }
            xyz_precision = 0;
        } else {
            // Entity
            Entity *entity = target.entity;
            x = entity->x;
            y = entity->y - entity->height_offset;
            z = entity->z;
            type = "Entity";
            const int type_id = entity->getEntityTypeId();
            type_info.push_back(format_type(type_id, misc_get_entity_type_name(entity).first));
            type_info.push_back("ID: " + safe_to_string(entity->id));
            if (entity->isMob()) {
                Mob *mob = (Mob *) entity;
                type_info.push_back("Health: " + safe_to_string(mob->health) + '/' + safe_to_string(mob->getMaxHealth()));
            }
            xyz_precision = debug_precision;
        }
        minecraft->command_server->pos_translator.to_float(x, y, z);
        info.push_back("");
        info.push_back("Target X: " + to_string_with_precision(x, xyz_precision));
        info.push_back("Target Y: " + to_string_with_precision(y, xyz_precision));
        info.push_back("Target Z: " + to_string_with_precision(z, xyz_precision));
        info.push_back("");
        info.push_back("Target: " + type);
        info.insert(info.end(), type_info.begin(), type_info.end());
    }
    // Return
    return info;
}

// Render Text With Background
static constexpr uint32_t debug_background_color = 0x90505050;
static constexpr int debug_text_color = 0xe0e0e0;
static void render_debug_line(Gui *gui, const std::string &line, int x, const int y, const bool right_aligned, const int pass) {
    // Draw Background
    const int width = gui->minecraft->font->width(line);
    if (width == 0) {
        return;
    }
    if (right_aligned) {
        const int screen_width = int(float(gui->minecraft->screen_width) * Gui::InvGuiScale);
        x = screen_width - x - width + 1;
    }
    const int x1 = x - 1;
    const int y1 = y - 1;
    const int x2 = x + width;
    const int y2 = y + line_height;
    if (pass == 0) {
        gui->fill(x1, y1, x2, y2, debug_background_color);
    }
    // Draw Text
    if (pass == 1) {
        gui->minecraft->font->draw(line, float(x), float(y), debug_text_color);
    }
}
// Draw Debug Information
static bool debug_info_shown = false;
static constexpr int debug_margin = 2;
static constexpr int debug_line_padding = 1;
static void render_debug_info(Gui *self, const std::vector<std::pair<bool, std::vector<std::string>>> &all_info, const int pass) {
    for (const std::pair<bool, std::vector<std::string>> &info : all_info) {
        int y = debug_margin;
        for (const std::string &line : info.second) {
            render_debug_line(self, line, debug_margin, y, info.first, pass);
            y += line_height;
            y += debug_line_padding;
        }
    }
}
static void Gui_renderDebugInfo_injection(MCPI_UNUSED Gui_renderDebugInfo_t original, Gui *self) {
    if (debug_info_shown) {
        // Get F3 Information
        const Minecraft *minecraft = self->minecraft;
        const std::vector<std::pair<bool, std::vector<std::string>>> all_info = {
            {false, get_debug_info_left(minecraft)},
            {true, get_debug_info_right(minecraft)}
        };

        // Draw Background
        Tesselator &t = Tesselator::instance;
        t.begin(GL_QUADS);
        t.voidBeginAndEndCalls(true);
        render_debug_info(self, all_info, 0);
        t.voidBeginAndEndCalls(false);
        media_glEnable(GL_BLEND);
        media_glDisable(GL_TEXTURE_2D);
        media_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        t.draw();
        media_glEnable(GL_TEXTURE_2D);
        media_glDisable(GL_BLEND);

        // Draw Text
        // This should already be batched.
        render_debug_info(self, all_info, 1);
    }
}

// Init
void init_f3() {
    if (feature_has("F3 Debug Information", server_disabled)) {
        overwrite_calls(Gui_renderDebugInfo, Gui_renderDebugInfo_injection);
        misc_run_on_game_key_press([](MCPI_UNUSED Minecraft *minecraft, const int key) {
            if (key == MC_KEY_F3) {
                debug_info_shown = !debug_info_shown;
                return true;
            } else {
                return false;
            }
        });
    }
    _init_f3_outlining();
}