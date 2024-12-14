#include <libreborn/env/env.h>
#include <libreborn/util/exec.h>
#include <libreborn/log.h>
#include <libreborn/env/flags.h>
#include <libreborn/env/servers.h>

// Clear Internal Variables
bool is_env_var_internal(const char *env) {
    return env[0] == '_';
}
void clear_internal_env_vars() {
#define ENV(name, ...) \
    if (is_env_var_internal(name##_ENV)) { \
        set_and_print_env(name##_ENV, nullptr); \
    }
#include <libreborn/env/list.h>
#undef ENV
}

// Set Environmental Variable
void setenv_safe(const char *name, const char *value) {
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

// Conversion
std::string obj_to_env_value(const std::string &obj) {
    return obj;
}
std::string obj_to_env_value(const float &obj) {
    return std::to_string(obj);
}
std::string obj_to_env_value(const Flags &obj) {
    return obj.to_string();
}
std::string obj_to_env_value(const ServerList &obj) {
    return obj.to_string();
}
void env_value_to_obj(std::string &out, const char *value) {
    out = value;
}
void env_value_to_obj(float &out, const char *value) {
    out = strtof(value, nullptr);
}
void env_value_to_obj(Flags &out, const char *value) {
    out.from_string(value);
}
void env_value_to_obj(ServerList &out, const char *value) {
    out.load(value);
}