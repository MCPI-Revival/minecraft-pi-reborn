#include "messages.h"

#include <symbols/Chunk.h>

// Empty layer Tracking
rebuilt_chunk_data::empty::empty() {
    for (bool &layer : layers) {
        layer = value;
    }
}
void rebuilt_chunk_data::empty::apply(Chunk *out) const {
    out->is_empty = value;
    for (int i = 0; i < num_layers; i++) {
        out->is_layer_empty[i] = layers[i];
    }
}
void rebuilt_chunk_data::empty::set(const int layer) {
    value = false;
    layers[layer] = value;
}

// Free Rebuilt Chunk Data
void _free_rebuilt_chunk_data(const rebuilt_chunk_data *chunk) {
    for (const VertexArray<CustomVertexFlat> *ptr : chunk->vertices) {
        delete ptr;
    }
    delete chunk;
}