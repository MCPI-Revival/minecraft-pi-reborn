#pragma once

#include <string>

#include "options/parser.h"

void bootstrap(const options_t &options);
void copy_sdk(const std::string &binary_directory, bool log_with_debug);
std::string bootstrap_mods(const std::string &binary_directory);
