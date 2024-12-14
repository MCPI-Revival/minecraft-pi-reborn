#pragma once

#include <string>

#include "../options/parser.h"
#include "../ui/frame.h"

#include <libreborn/env/flags.h>
#include <libreborn/env/servers.h>

// Default Configuration
#define DEFAULT_USERNAME "StevePi"
#define DEFAULT_RENDER_DISTANCE "Short"
#define AUTO_GUI_SCALE 0

// State
struct State {
    State();
    // Methods
    void update(bool save);
    bool operator==(const State &other) const;
    // Properties
    std::string username;
    std::string render_distance;
    ServerList servers;
    float gui_scale;
    Flags flags;
};

// UI
struct ConfigurationUI final : Frame {
    explicit ConfigurationUI(State &state_, bool &save_settings_);
    int render() override;
private:
    // Bottom Row
    [[nodiscard]] int get_render_distance_index() const;
    [[nodiscard]] int draw_bottom() const;
    // General
    void draw_main() const;
    // Advanced
    void draw_advanced() const;
    static void draw_category(FlagNode &category);
    // Server List
    void draw_servers() const;
    void draw_server_list() const;
    // State
    const State original_state;
    State &state;
    bool &save_settings;
};

// Handle Non-Launch Commands
void handle_non_launch_client_only_commands(const options_t &options);

// Configure Client Options
void configure_client(const options_t &options);