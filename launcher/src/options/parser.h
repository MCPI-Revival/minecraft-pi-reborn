#pragma once

#define OPTION(name, ...) bool name;
struct options_t {
#include "list.h"
};
#undef OPTION
options_t parse_options(int argc, char *argv[]);