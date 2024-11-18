#include <libreborn/env.h>
#include <libreborn/exec.h>
#include <libreborn/log.h>

// Define Constants
#define ENV(name, ...) const char *const name##_ENV = #name;
#include <libreborn/env-list.h>
#undef ENV

// Clear Internal Variables
bool is_env_var_internal(const char *env) {
    return env[0] == '_';
}
void clear_internal_env_vars() {
    DEBUG("Clearing Internal Environmental Variables...");
#define ENV(name, ...) \
    if (is_env_var_internal(name##_ENV)) { \
        set_and_print_env(name##_ENV, nullptr); \
    }
#include <libreborn/env-list.h>
#undef ENV
}

// Set Environmental Variable
static void setenv_safe(const char *name, const char *value) {
    if (value != nullptr) {
        setenv(name, value, 1);
    } else {
        unsetenv(name);
    }
}
void set_and_print_env(const char *name, const char *value) {
    // Set The Value
    setenv_safe(name, value);

    // Print New Value
    DEBUG("Set %s = %s", name, value != NULL ? value : "(unset)");
}