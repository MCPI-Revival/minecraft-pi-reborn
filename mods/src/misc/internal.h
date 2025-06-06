#pragma once

MCPI_INTERNAL void _init_misc_logging();
MCPI_INTERNAL void _init_misc_api();
MCPI_INTERNAL void _init_misc_graphics();
MCPI_INTERNAL void _init_misc_ui();
MCPI_INTERNAL void _init_misc_home();

template <typename... Args>
static void nop(MCPI_UNUSED Args... args) {
}