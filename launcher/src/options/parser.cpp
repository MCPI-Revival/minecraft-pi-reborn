#include <argp.h>

#include "parser.h"

// Globals
const char *argp_program_version = "Reborn v" MCPI_VERSION;
const char *argp_program_bug_address = "<" MCPI_DISCORD_INVITE ">";
static char doc[] = "Minecraft: Pi Edition Modding Project";

// Options
#define OPTION(ignored, name, key, doc) {name, key, nullptr, 0, doc, 0},
static argp_option options_data[] = {
#include "option-list.h"
    {nullptr, 0, nullptr, 0, nullptr, 0}
};
#undef OPTION

// Parse Options
#define OPTION(name, ignored, key, ...) \
    case key: \
        options->name = true; \
        break;
static error_t parse_opt(int key, __attribute__((unused)) char *arg, argp_state *state) {
    options_t *options = (options_t *) state->input;
    switch (key) {
#include "option-list.h"
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}
#undef OPTION
static argp argp = {options_data, parse_opt, nullptr, doc, nullptr, nullptr, nullptr};
options_t parse_options(int argc, char *argv[]) {
    options_t options = {};
    argp_parse(&argp, argc, argv, 0, nullptr, &options);
    return options;
}