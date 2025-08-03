#pragma once

#include <string>

// Define Variables
#define ENV(name, ...) constexpr const char *name##_ENV = #name;
#include "list.h"
#undef ENV

// Internal Variables
bool is_env_var_internal(const char *env);
void clear_internal_env_vars();

// Set Environmental Variable
void setenv_safe(const char *name, const char *value);
void set_and_print_env(const char *name, const char *value);

// Get Value
const char *require_env(const char *name);
bool is_env_set(const char *name);

// Convert Variable To Value And Vice-Versa
struct Flags;
struct ServerList;
#define overload(type) \
    std::string obj_to_env_value(const type &obj); \
    void env_value_to_obj(type &out, const char *value)
overload(std::string);
overload(float);
overload(Flags);
overload(ServerList);
#undef overload