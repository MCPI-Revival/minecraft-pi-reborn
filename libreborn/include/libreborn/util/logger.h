#pragma once

#include <cstdio>
#include <optional>

#include "io.h"

// Log File
MCPI_REBORN_UTIL_PUBLIC FILE *reborn_get_log_file();
MCPI_REBORN_UTIL_PUBLIC void reborn_init_log(const std::optional<HANDLE> &fd);

// Debug Logging
MCPI_REBORN_UTIL_PUBLIC FILE *reborn_get_debug_file();

// Crash Message
// This is not thread-safe.
MCPI_REBORN_UTIL_PUBLIC const char *reborn_get_crash_message(const char *reason);