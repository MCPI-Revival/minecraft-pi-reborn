#include <argp.h>

#include <libreborn/config.h>
#include <libreborn/env/env.h>
#include <trampoline/types.h>

#include "parser.h"

// Globals
static std::string version = std::string("Reborn ") + reborn_get_fancy_version();
const char *argp_program_version = version.c_str();
static std::string bug_address = std::string("<") + reborn_config.extra.discord_invite + '>';
const char *argp_program_bug_address = bug_address.c_str();
static char doc[] = "Minecraft: Pi Edition Modding Project";

// Options
static int env_key = -100;
static argp_option options_data[] = {
    {nullptr, 0, nullptr, 0, "Game Options:", 0},
#define OPTION(ignored, name, key, doc) {name, key, nullptr, 0, doc, 0},
#include "list.h"
#undef OPTION
    {nullptr, 0, nullptr, 0, "Environmental Variables:", 0},
#define ENV(name, doc, ...) {#name, env_key--, nullptr, OPTION_DOC | OPTION_NO_USAGE | (is_env_var_internal(name##_ENV) ? OPTION_HIDDEN : 0), doc, 0},
#include <libreborn/env/list.h>
#ifdef MCPI_BUILD_RUNTIME
#include <trampoline/env-list.h>
#endif
#undef ENV
    {nullptr, 0, nullptr, 0, "Help Options:", -1},
    {nullptr, 0, nullptr, 0, nullptr, 0}
};

// Parse Options
#define OPTION(name, ignored, key, ...) \
    case key: options->name = true; break;
static error_t parse_opt(const int key, MCPI_UNUSED char *arg, argp_state *state) {
    options_t *options = (options_t *) state->input;
    switch (key) {
#include "list.h"
        default: return ARGP_ERR_UNKNOWN;
    }
    return 0;
}
#undef OPTION
static argp argp = {options_data, parse_opt, nullptr, doc, nullptr, nullptr, nullptr};
options_t parse_options(const int argc, char *argv[]) {
    options_t options = {};
    argp_parse(&argp, argc, argv, 0, nullptr, &options);
    return options;
}