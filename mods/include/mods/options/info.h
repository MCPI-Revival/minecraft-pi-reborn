#pragma once

#include <string>

#define CHANGELOG_FILE "CHANGELOG.md"

extern "C" {
void open_url(const std::string &url);
}