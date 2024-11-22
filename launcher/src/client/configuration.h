#pragma once

#include <string>

#include "../options/parser.h"
#include "cache.h"
#include "../ui/frame.h"

#include <libreborn/flags.h>

// Default Configuration
#define DEFAULT_USERNAME "StevePi"
#define DEFAULT_RENDER_DISTANCE "Short"

// State
struct State {
    explicit State(const launcher_cache &cache);
    // Methods
    void update(bool save);
    bool operator==(const State &other) const;
    // Properties
    std::string username;
    std::string render_distance;
    Flags flags;
};

// UI
struct ConfigurationUI final : Frame {
    explicit ConfigurationUI(State &state_, bool &save_settings_);
    int render() override;
private:
    void update_render_distance();
    int draw_bottom();
    void draw_main();
    void draw_advanced() const;
    static void draw_category(FlagNode &category);
    const State default_state;
    const State original_state;
    State &state;
    bool &save_settings;
    int render_distance_index;
};

// Handle Non-Launch Commands
void handle_non_launch_client_only_commands(const options_t &options);

// Configure Client Options
void configure_client(const options_t &options);