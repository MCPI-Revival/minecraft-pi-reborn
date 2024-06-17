#include <libreborn/env.h>
#include <libreborn/exec.h>

// Define Constants
#define ENV(name, ...) const char *name##_ENV = #name;
#include <libreborn/env_list.h>
#undef ENV

// Clear Internal Variables
int is_env_var_internal(const char *env) {
    return env[0] == '_';
}
void clear_internal_env_vars() {
#define ENV(name, ...) \
    if (is_env_var_internal(name##_ENV)) { \
        set_and_print_env(name##_ENV, NULL); \
    }
#include <libreborn/env_list.h>
#undef ENV
}