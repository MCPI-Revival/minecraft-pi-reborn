#pragma once

// Flags In Server-Mode
static constexpr bool server_enabled = true;
static constexpr bool server_disabled = false;
extern bool feature_server_flags_set;
#define FLAG(name) extern bool server_##name
#include "server.h"
#undef FLAG
#define server_is_not_vanilla_compatible (!server_is_vanilla_compatible)

// Check If The Flag Is Enabled
extern "C" {
bool feature_has(const char *name, bool enabled_in_server_mode);
}
