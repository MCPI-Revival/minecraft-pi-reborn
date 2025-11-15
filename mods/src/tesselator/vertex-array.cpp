#include <libreborn/log.h>

#include <mods/tesselator/tesselator.h>

// Maximum Vertex Count
static constexpr int max_vertices = 524288;

// Constructor
template <typename T>
VertexArray<T>::VertexArray(const int max_size) {
    data = new T[max_size];
    clear();
}
template <typename T>
VertexArray<T>::VertexArray():
    VertexArray(max_vertices) {}
template <typename T>
VertexArray<T>::~VertexArray() {
    delete[] data;
}

// Methods
template <typename T>
void VertexArray<T>::push_back(const T &value) {
    if (size >= max_vertices) {
        IMPOSSIBLE();
    }
    data[size++] = value;
}
template <typename T>
void VertexArray<T>::clear() {
    size = 0;
}

// Copy
template<typename T>
const VertexArray<T> *VertexArray<T>::copy() const {
    VertexArray *out = new VertexArray(size);
    out->size = size;
    memcpy(out->data, data, size * sizeof(T));
    return out;
}
template<typename T>
void VertexArray<T>::receive(const VertexArray *other) {
    memcpy(data, other->data, other->size * sizeof(T));
    size = other->size;
    delete other;
}

// Instantiate Templates
template struct VertexArray<CustomVertexFlat>;
template struct VertexArray<CustomVertexShaded>;