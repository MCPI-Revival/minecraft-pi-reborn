#include "media.h"

#include <optional>
#include <GLES/gl.h>

// Free Framebuffer
static std::optional<GLuint> fbo;
void _media_cleanup_offscreen_render() {
    if (fbo.has_value()) {
        media_glDeleteFramebuffersEXT(1, &fbo.value());
    }
}

// Create Framebuffer
static GLuint fbo_texture;
static void setup_fbo(const GLuint texture) {
    if (!glfw_window) {
        IMPOSSIBLE();
    }
    // Create Framebuffer
    _media_cleanup_offscreen_render();
    GLuint fbo_id;
    media_glGenFramebuffersEXT(1, &fbo_id);
    media_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_id);
    // Attach Texture
    media_glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texture, 0);
    // Check Status
    const GLenum status = media_glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
        IMPOSSIBLE();
    }
    media_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    // Store Values
    fbo = fbo_id;
    fbo_texture = texture;
}

// Offscreen Rendering
void media_begin_offscreen_render(const unsigned int texture) {
    if (!fbo.has_value() || fbo_texture != texture) {
        setup_fbo(texture);
    }
    media_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo.value());
}
void media_end_offscreen_render() {
    media_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}