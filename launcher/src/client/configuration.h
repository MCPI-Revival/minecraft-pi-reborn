#pragma once

#include <string>
#include <functional>

#include "../options/parser.h"

// Defaults
#define DEFAULT_USERNAME "StevePi"
#define DEFAULT_RENDER_DISTANCE "Short"

// Feature Flags
std::string strip_feature_flag_default(const std::string& flag, bool *default_ret);
void load_available_feature_flags(const std::function<void(std::string)> &callback);

// Handle Non-Launch Commands
void handle_non_launch_client_only_commands(const options_t &options);

// Check Environment
void check_environment_client();

// Configure Client Options
void configure_client(const options_t &options);