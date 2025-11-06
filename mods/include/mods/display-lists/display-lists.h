#pragma once

struct LevelRenderer;
struct Level;
struct LevelSource;

extern bool display_lists_enabled_for_chunk_rendering;

extern "C" void LevelRenderer_renderSameAsLast(LevelRenderer *self, float delta);
extern "C" Level *get_level_from_cached_level_source(const LevelSource *level_source);
