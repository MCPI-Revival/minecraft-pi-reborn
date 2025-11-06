#pragma once

#include <symbols/minecraft.h>

extern bool display_lists_enabled_for_chunk_rendering;

extern "C" {
void LevelRenderer_renderSameAsLast(LevelRenderer *self, float delta);
}
