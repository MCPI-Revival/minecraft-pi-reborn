#pragma once

__attribute__((visibility("internal"))) void _init_misc_logging();
__attribute__((visibility("internal"))) void _init_misc_api();
__attribute__((visibility("internal"))) void _init_misc_graphics();
__attribute__((visibility("internal"))) void _init_misc_ui();
__attribute__((visibility("internal"))) void _init_misc_tinting();

template <typename... Args>
static void nop(__attribute__((unused)) Args... args) {
}