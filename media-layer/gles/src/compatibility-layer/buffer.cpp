#include <unordered_map>

#include <GLES/gl.h>

#include "../passthrough.h"

// Store Buffers
static std::unordered_map<GLuint, GLuint> buffers_map;
// Get Buffer
GL_FUNC(glGenBuffers, void, (GLsizei n, GLuint *buffers));
static GLuint get_real_buffer(GLuint fake_buffer) {
    if (buffers_map.count(fake_buffer) > 0) {
        return buffers_map[fake_buffer];
    } else {
        GLuint new_buffer;
        real_glGenBuffers()(1, &new_buffer);
        buffers_map[fake_buffer] = new_buffer;
        return get_real_buffer(fake_buffer);
    }
}

// Convert Fake Buffers To Real Buffers When Calling GL
GL_FUNC(glBindBuffer, void, (GLenum target, GLuint buffer));
void glBindBuffer(GLenum target, GLuint buffer) {
    real_glBindBuffer()(target, get_real_buffer(buffer));
}
GL_FUNC(glDeleteBuffers, void, (GLsizei n, const GLuint *buffers));
void glDeleteBuffers(GLsizei n, const GLuint *buffers) {
    for (int i = 0; i < n; i++) {
        if (buffers_map.count(buffers[i]) > 0) {
            real_glDeleteBuffers()(1, &buffers_map[i]);
            buffers_map.erase(buffers[i]);
        }
    }
}
