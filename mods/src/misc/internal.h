#pragma once

void _init_misc_logging();
void _init_misc_api();
void _init_misc_graphics();
void _init_misc_ui();
void _init_misc_home();
void _init_misc_item_rendering();
void _init_misc_entity_rendering();
void _init_misc_chest_rendering();
void _init_misc_daynight_cycle();

template <typename... Args>
static void nop(MCPI_UNUSED Args... args) {
}