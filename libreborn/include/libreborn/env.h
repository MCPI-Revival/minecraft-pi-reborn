#pragma once

#include <string>

#define ENV(name, ...) extern const char *const name##_ENV;
#include "env-list.h"
#undef ENV

bool is_env_var_internal(const char *env);
void clear_internal_env_vars();

// Set Environmental Variable
void set_and_print_env(const char *name, const char *value);

// Convert Variable To Value And Vice-Versa
struct Flags;
std::string obj_to_env_value(const std::string &obj);
std::string obj_to_env_value(const float &obj);
std::string obj_to_env_value(const Flags &obj);
void env_value_to_obj(std::string &out, const char *value);
void env_value_to_obj(float &out, const char *value);
void env_value_to_obj(Flags &out, const char *value);