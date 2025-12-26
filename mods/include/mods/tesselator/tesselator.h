#pragma once

#include <optional>

#include <GLES/gl.h>
#include <symbols/Vec3.h>

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
    explicit VertexArray(int max_size);
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
    void clear(bool full);
    void voidBeginAndEndCalls(bool x);
    bool active;
    GLenum mode;
    bool void_begin_end;
    bool are_vertices_flat;

    // Next Vertex Information
    Vec3 offset;
    float scale_x;
    float scale_y;
    void reset_scale();
    std::optional<UV> uv;
    bool no_color;
    std::optional<uint> color;
    std::optional<uint32_t> normal;

    // Buffers
    bool has_buffer;
    GLuint buffer;
    GLsizeiptr buffer_size;
};

// Method
void advanced_tesselator_enable();
CustomTesselator &advanced_tesselator_get();