#pragma once

#include <string>

#include "../options/parser.h"
#include "../ui/frame.h"

#include <libreborn/flags.h>
#include <libreborn/servers.h>

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
    float gui_scale;
    Flags flags;
};

// UI
struct ConfigurationUI final : Frame {
    explicit ConfigurationUI(State &state_, bool &save_settings_);
    int render() override;
private:
    // Bottom Row
    int get_render_distance_index() const;
    int draw_bottom(bool hide_reset_revert) const;
    // General
    void draw_main() const;
    // Advanced
    void draw_advanced() const;
    static void draw_category(FlagNode &category);
    // Server List
    bool are_servers_unsaved() const;
    void draw_servers();
    void draw_server_list();
    // State
    const State original_state;
    State &state;
    bool &save_settings;
    // Server List
    ServerList last_saved_servers;
    ServerList servers;
};

// Handle Non-Launch Commands
void handle_non_launch_client_only_commands(const options_t &options);

// Configure Client Options
void configure_client(const options_t &options);