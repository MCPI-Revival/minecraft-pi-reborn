#pragma once

#include <string>
#include <optional>

// Define Variables
#define ENV(name, ...) constexpr const char *name##_ENV = #name;
#include "list.h"
#undef ENV

// Internal Variables
MCPI_REBORN_UTIL_PUBLIC bool is_env_var_internal(const char *env);
MCPI_REBORN_UTIL_PUBLIC void clear_internal_env_vars();

// Set Environmental Variable
MCPI_REBORN_UTIL_PUBLIC void setenv_safe(const char *name, const char *value);
MCPI_REBORN_UTIL_PUBLIC void set_and_print_env(const char *name, const char *value);

// Get Value
MCPI_REBORN_UTIL_PUBLIC std::optional<std::string> getenv_safe(const char *name);
MCPI_REBORN_UTIL_PUBLIC std::string require_env(const char *name);

// Convert Variable To Value And Vice-Versa
struct Flags;
struct ServerList;
#define overload(type) \
    MCPI_REBORN_UTIL_PUBLIC std::string obj_to_env_value(const type &obj); \
    MCPI_REBORN_UTIL_PUBLIC void env_value_to_obj(type &out, const char *value)
overload(std::string);
overload(float);
overload(unsigned short);
overload(Flags);
overload(ServerList);
#undef overload