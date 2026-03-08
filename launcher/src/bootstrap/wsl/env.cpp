#include "../bootstrap.h"
#include "../../util/util.h"

#include <libreborn/env/env.h>
#include <libreborn/util/exec.h>

#include <trampoline/host.h>

// Set WSLENV
static void set_wslenv() {
    // Get List Of Set Variables
    std::vector<std::string> variables;
#define ENV(name, _, flags) \
    if ((flags)[0] != '[') { \
        variables.push_back(name##_ENV + std::string(flags)); \
    }
#include <libreborn/env/list.h>
#undef ENV
    // https://devblogs.microsoft.com/commandline/share-environment-vars-between-wsl-and-windows/
    const char *wslenv = "WSLENV";
    std::string value = getenv_safe(wslenv).value_or("");
    for (const std::string &it : variables) {
        if (!value.empty()) {
            value += ':';
        }
        value += it;
    }
    set_and_print_env(wslenv, value.c_str());
}

// Set Flags
static void set_wsl_flags() {
    constexpr const char *flags[] = {WSL_FLAGS, nullptr};
    const std::string flags_str = make_cmd(flags);
    set_and_print_env(_MCPI_WSL_FLAGS_ENV, flags_str.c_str());
}

// Configure WSL
void configure_wsl() {
    set_wslenv();
    set_wsl_flags();
}