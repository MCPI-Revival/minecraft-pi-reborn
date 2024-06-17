#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define ENV(name, ...) extern const char *name##_ENV;
#include "env-list.h"
#undef ENV

int is_env_var_internal(const char *env);
void clear_internal_env_vars();

#ifdef __cplusplus
}
#endif