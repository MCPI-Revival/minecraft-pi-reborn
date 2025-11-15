#pragma once

struct LevelRenderer;
struct BiomeSource;

extern "C" {
void LevelRenderer_renderSameAsLast(LevelRenderer *self, float delta);
BiomeSource *get_biome_source_on_chunk_rebuild_thread();
}