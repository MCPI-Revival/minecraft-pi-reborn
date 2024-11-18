#pragma once

#define ENV(name, ...) extern const char *const name##_ENV;
#include "env-list.h"
#undef ENV

bool is_env_var_internal(const char *env);
void clear_internal_env_vars();

// Set Environmental Variable
void set_and_print_env(const char *name, const char *value);