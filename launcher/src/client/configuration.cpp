#include <sstream>

#include <libreborn/env.h>

#include "configuration.h"
#include "cache.h"

// State
State::State(): flags("") {
    username = DEFAULT_USERNAME;
    render_distance = DEFAULT_RENDER_DISTANCE;
    gui_scale = AUTO_GUI_SCALE;
    flags = Flags::get();
}
template <typename T>
static void update_from_env(const char *env, T &value, const bool save) {
    if (save) {
        set_and_print_env(env, obj_to_env_value(value).c_str());
    } else {
        const char *env_value = getenv(env);
        if (env_value != nullptr) {
            env_value_to_obj(value, env_value);
        }
    }
}
void State::update(const bool save) {
    update_from_env(MCPI_FEATURE_FLAGS_ENV, flags, save);
    update_from_env(MCPI_USERNAME_ENV, username, save);
    update_from_env(MCPI_RENDER_DISTANCE_ENV, render_distance, save);
    update_from_env(MCPI_GUI_SCALE_ENV, gui_scale, save);
}
bool State::operator==(const State &other) const {
    std::ostringstream one;
    write_cache(one, *this);
    std::ostringstream two;
    write_cache(two, other);
    return one.str() == two.str();
}

// Handle Non-Launch Commands
void handle_non_launch_client_only_commands(const options_t &options) {
    // Print Available Feature Flags
    if (options.print_available_feature_flags) {
        const Flags flags = Flags::get();
        flags.print();
        exit(EXIT_SUCCESS);
    }
    // Wipe Cache If Needed
    if (options.wipe_cache) {
        wipe_cache();
        exit(EXIT_SUCCESS);
    }
}

// Configure Client Options
void configure_client(const options_t &options) {
    // Load Cache
    State state;
    bool save_settings = !options.no_cache;
    if (save_settings) {
        state = load_cache();
    }

    // Read From Environment
    state.update(false);

    // Show UI
    if (!options.use_default) {
        ConfigurationUI *ui = new ConfigurationUI(state, save_settings);
        const int ret = ui->run();
        delete ui;
        if (ret <= 0) {
            // Cancel Launch
            exit(EXIT_SUCCESS);
        }
    }

    // Save Cache
    if (save_settings) {
        save_cache(state);
    }

    // Update Environment
    state.update(true);
}
