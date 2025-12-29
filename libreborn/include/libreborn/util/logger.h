#pragma once

#include <cstdio>
#include <optional>

#include "io.h"

// Log File
FILE *reborn_get_log_file();
void reborn_init_log(const std::optional<HANDLE> &fd);

// Debug Logging
FILE *reborn_get_debug_file();

// Crash Message
// This is not thread-safe.
const char *reborn_get_crash_message(const char *reason);