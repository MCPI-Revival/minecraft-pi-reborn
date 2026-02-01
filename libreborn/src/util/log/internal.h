#pragma once

#include <optional>
#include <cstdio>

// Current Log File
MCPI_INTERNAL extern std::optional<FILE *> log_file;
MCPI_INTERNAL void close_log_file();

// Check If The Current Process Is The Logger
MCPI_INTERNAL bool is_logger_process();