#include "media.h"

#include <GLES/gl.h>

// Offscreen Rendering
static GLuint fbo;
void media_begin_offscreen_render(const unsigned int texture) {
    if (!glfw_window) {
        IMPOSSIBLE();
    }
    // Create Framebuffer
    media_glGenFramebuffersEXT(1, &fbo);
    media_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
    // Attach Texture
    media_glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texture, 0);
    // Check Status
    const GLenum status = media_glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
        IMPOSSIBLE();
    }
}
void media_end_offscreen_render() {
    // Destroy Framebuffer
    media_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    media_glDeleteFramebuffersEXT(1, &fbo);
}