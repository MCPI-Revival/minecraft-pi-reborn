#pragma once

#include <optional>

#include <GLES/gl.h>
#include <symbols/minecraft.h>

// Structures
struct UV {
    float u;
    float v;
};
struct CustomVertexFlat {
    // This Matches Vanilla Vertices
    Vec3 pos;
    UV uv;
    GLuint color;
};
struct CustomVertexShaded {
    // This Adds Normal Information
    CustomVertexFlat base;
    GLuint normal;
};

// Vertex Array
template <typename T>
struct VertexArray {
    VertexArray();
    ~VertexArray();
    void push_back(const T &value);
    void clear();
    [[nodiscard]] const VertexArray *copy() const;
    void receive(const VertexArray *other);
    // Properties
    T *data;
    int size;
};
extern template struct VertexArray<CustomVertexFlat>;
extern template struct VertexArray<CustomVertexShaded>;

// Advanced Tesselator
struct CustomTesselator {
    // Constructor
    explicit CustomTesselator(bool create_buffers);
    ~CustomTesselator();

    // Vertices
    VertexArray<CustomVertexShaded> vertices;
    VertexArray<CustomVertexFlat> vertices_flat;

    // State
    void clear();
    bool active;
    GLenum mode;
    bool void_begin_end;
    bool are_vertices_flat;
    bool enable_real_quads;

    // Next Vertex Information
    Vec3 offset;
    float scale_x;
    float scale_y;
    void reset_scale();
    std::optional<UV> uv;
    bool no_color;
    std::optional<uint> color;
    std::optional<uint32_t> normal;
    int quad_to_triangle_tracker;

    // Buffers
    GLsizei buffer_count;
    GLuint *buffers;
    uint next_buffer_index;
};

// Method
void advanced_tesselator_enable();
CustomTesselator &advanced_tesselator_get();