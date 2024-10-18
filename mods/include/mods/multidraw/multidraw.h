#pragma once

#include <symbols/minecraft.h>

extern int multidraw_vertex_size;
extern "C" {
void LevelRenderer_renderSameAsLast(LevelRenderer *self, float delta);
}
