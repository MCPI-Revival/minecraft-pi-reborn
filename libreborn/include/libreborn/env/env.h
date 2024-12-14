#pragma once

#include <string>

#define ENV(name, ...) constexpr const char *name##_ENV = #name;
#include "list.h"
#undef ENV

bool is_env_var_internal(const char *env);
void clear_internal_env_vars();

// Set Environmental Variable
void setenv_safe(const char *name, const char *value);
void set_and_print_env(const char *name, const char *value);

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