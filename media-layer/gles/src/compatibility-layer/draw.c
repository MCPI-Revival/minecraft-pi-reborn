#include "state.h"
#include "passthrough.h"

#include <GLES/gl.h>

#include <libreborn/libreborn.h>

// Shaders
#define REAL_GL_FRAGMENT_SHADER 0x8b30
#define REAL_GL_VERTEX_SHADER 0x8b31
#define REAL_GL_INFO_LOG_LENGTH 0x8b84
#define REAL_GL_COMPILE_STATUS 0x8b81
GL_FUNC(glUseProgram, void, (GLuint program));
GL_FUNC(glGetUniformLocation, GLint, (GLuint program, const GLchar *name));
GL_FUNC(glUniformMatrix4fv, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value));
GL_FUNC(glUniform1i, void, (GLint location, GLint v0));
GL_FUNC(glUniform1f, void, (GLint location, GLfloat v0));
GL_FUNC(glUniform4f, void, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3));
GL_FUNC(glGetAttribLocation, GLint, (GLuint program, const GLchar *name));
GL_FUNC(glEnableVertexAttribArray, void, (GLuint index));
GL_FUNC(glDisableVertexAttribArray, void, (GLuint index));
GL_FUNC(glVertexAttribPointer, void, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer));
GL_FUNC(glVertexAttrib3f, void, (GLuint index, GLfloat v0, GLfloat v1, GLfloat v2));
GL_FUNC(glVertexAttrib4f, void, (GLuint index, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3));
GL_FUNC(glCreateShader, GLuint, (GLenum type));
GL_FUNC(glShaderSource, void, (GLuint shader, GLsizei count, const GLchar *const *string, const GLint *length));
GL_FUNC(glCompileShader, void, (GLuint shader));
GL_FUNC(glCreateProgram, GLuint, ());
GL_FUNC(glAttachShader, void, (GLuint program, GLuint shader));
GL_FUNC(glLinkProgram, void, (GLuint program));
GL_FUNC(glGetShaderiv, void, (GLuint shader, GLenum pname, GLint *params));
GL_FUNC(glGetShaderInfoLog, void, (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog));

// Compile Shader
static void log_shader(GLuint shader, const char *name) {
    // Log
    GLint log_length = 0;
    real_glGetShaderiv()(shader, REAL_GL_INFO_LOG_LENGTH, &log_length);
    GLchar *log = malloc(log_length * sizeof (GLchar));
    ALLOC_CHECK(log);
    real_glGetShaderInfoLog()(shader, log_length, &log_length, log);
    if (log_length > 0) {
        if (log_length > 1 && log[log_length - 1] == '\n') {
            log[log_length - 1] = '\0';
        }
        DEBUG("%s Shader Compile Log: %s", name, log);
    }
    free(log);

    // Check Status
    GLint is_compiled = 0;
    real_glGetShaderiv()(shader, REAL_GL_COMPILE_STATUS, &is_compiled);
    if (!is_compiled) {
        ERR("Failed To Compile %s Shader", name);
    }
}
static GLuint compile_shader(const char *vertex_shader_text, const char *fragment_shader_text) {
    // Vertex Shader
    const GLuint vertex_shader = real_glCreateShader()(REAL_GL_VERTEX_SHADER);
    real_glShaderSource()(vertex_shader, 1, &vertex_shader_text, NULL);
    real_glCompileShader()(vertex_shader);
    log_shader(vertex_shader, "Vertex");

    // Fragment Shader
    const GLuint fragment_shader = real_glCreateShader()(REAL_GL_FRAGMENT_SHADER);
    real_glShaderSource()(fragment_shader, 1, &fragment_shader_text, NULL);
    real_glCompileShader()(fragment_shader);
    log_shader(fragment_shader, "Fragment");

    // Link
    GLuint program = real_glCreateProgram()();
    real_glAttachShader()(program, vertex_shader);
    real_glAttachShader()(program, fragment_shader);
    real_glLinkProgram()(program);

    // Return
    return program;
}

// Shader
static GLuint get_shader() {
    static GLuint program = 0;
    if (program == 0) {
        static const char *vertex_shader_text =
            "#version 100\n"
            "precision mediump float;\n"
            // Matrices
            "uniform mat4 u_projection;\n"
            "uniform mat4 u_model_view;\n"
            "uniform mat4 u_texture;\n"
            // Texture
            "attribute vec3 a_vertex_coords;\n"
            "attribute vec2 a_texture_coords;\n"
            "varying vec4 v_texture_pos;\n"
            // Color
            "attribute vec4 a_color;\n"
            "varying vec4 v_color;\n"
            // Fog
            "varying vec4 v_fog_eye_position;\n"
            // Main
            "void main() {\n"
            "    v_texture_pos = u_texture * vec4(a_texture_coords.xy, 0.0, 1.0);\n"
            "    gl_Position = u_projection * u_model_view * vec4(a_vertex_coords.xyz, 1.0);\n"
            "    v_color = a_color;\n"
            "    v_fog_eye_position = u_model_view * vec4(a_vertex_coords.xyz, 1.0);\n"
            "}";
        static const char *fragment_shader_text =
            "#version 100\n"
            "precision mediump float;\n"
            // Texture
            "uniform bool u_has_texture;"
            "uniform sampler2D u_texture_unit;\n"
            // Color
            "varying vec4 v_color;\n"
            "varying vec4 v_texture_pos;\n"
            // Alpha Test
            "uniform bool u_alpha_test;\n"
            // Fog
            "uniform bool u_fog;\n"
            "uniform vec4 u_fog_color;\n"
            "uniform bool u_fog_is_linear;\n"
            "uniform float u_fog_start;\n"
            "uniform float u_fog_end;\n"
            "varying vec4 v_fog_eye_position;\n"
            // Main
            "void main(void) {\n"
            "    gl_FragColor = v_color;\n"
            "    if (u_has_texture) {\n"
            "        gl_FragColor *= texture2D(u_texture_unit, v_texture_pos.xy);\n"
            "    }\n"
            "    if (u_alpha_test && gl_FragColor.a <= 0.1) {\n"
            "        discard;\n"
            "    }\n"
            "    if (u_fog) {\n"
            "        float fog_factor;\n"
            "        if (u_fog_is_linear) {\n"
            "            fog_factor = (u_fog_end - length(v_fog_eye_position)) / (u_fog_end - u_fog_start);\n"
            "        } else {\n"
            "            fog_factor = exp(-u_fog_start * length(v_fog_eye_position));\n"
            "        }\n"
            "        gl_FragColor = mix(gl_FragColor, u_fog_color, 1.0 - clamp(fog_factor, 0.0, 1.0));\n"
            "    }\n"
            "}";
        program = compile_shader(vertex_shader_text, fragment_shader_text);
    }
    return program;
}

// Shader Switching
static void use_shader(GLuint program) {
    static GLuint current_program = 0;
    if (current_program != program) {
        real_glUseProgram()(program);
        current_program = program;
    }
}

// Array Pointer Drawing
GL_FUNC(glDrawArrays, void, (GLenum mode, GLint first, GLsizei count));
void glDrawArrays(GLenum mode, GLint first, GLsizei count) {
    // Verify
    if (gl_state.array_pointers.vertex.size != 3 || !gl_state.array_pointers.vertex.enabled || gl_state.array_pointers.vertex.type != GL_FLOAT) {
        ERR("Unsupported Vertex Conifguration");
    }

    // Check Mode
    int use_color_pointer = gl_state.array_pointers.color.enabled;
    if (use_color_pointer && (gl_state.array_pointers.color.size != 4 || gl_state.array_pointers.color.type != GL_UNSIGNED_BYTE)) {
        ERR("Unsupported Color Conifguration");
    }
    int use_texture = gl_state.texture_2d && gl_state.array_pointers.tex_coord.enabled;
    if (use_texture && (gl_state.array_pointers.tex_coord.size != 2 || gl_state.array_pointers.tex_coord.type != GL_FLOAT)) {
        ERR("Unsupported Texture Conifguration");
    }

    // Load Shader
    GLuint program = get_shader();
    use_shader(program);

    // Projection Matrix
    GLint u_projection_handle = real_glGetUniformLocation()(program, "u_projection");
    matrix_t *p = &gl_state.matrix_stacks.projection.stack[gl_state.matrix_stacks.projection.i];
    real_glUniformMatrix4fv()(u_projection_handle, 1, 0, (GLfloat *) &p->data[0][0]);

    // Model View Matrix
    GLint u_model_view_handle = real_glGetUniformLocation()(program, "u_model_view");
    p = &gl_state.matrix_stacks.model_view.stack[gl_state.matrix_stacks.model_view.i];
    real_glUniformMatrix4fv()(u_model_view_handle, 1, 0, (GLfloat *) &p->data[0][0]);

    // Has Texture
    GLint u_has_texture_handle = real_glGetUniformLocation()(program, "u_has_texture"); \
    real_glUniform1i()(u_has_texture_handle, use_texture); \

    // Texture Matrix
    GLint u_texture_handle = real_glGetUniformLocation()(program, "u_texture");
    p = &gl_state.matrix_stacks.texture.stack[gl_state.matrix_stacks.texture.i];
    real_glUniformMatrix4fv()(u_texture_handle, 1, 0, (GLfloat *) &p->data[0][0]);

    // Texture Unit
    GLint u_texture_unit_handle = real_glGetUniformLocation()(program, "u_texture_unit");
    real_glUniform1i()(u_texture_unit_handle, 0);

    // Alpha Test
    GLint u_alpha_test_handle = real_glGetUniformLocation()(program, "u_alpha_test");
    real_glUniform1i()(u_alpha_test_handle, gl_state.alpha_test);

    // Color
    GLint a_color_handle = real_glGetAttribLocation()(program, "a_color");
    if (use_color_pointer) {
        real_glVertexAttribPointer()(a_color_handle, gl_state.array_pointers.color.size, gl_state.array_pointers.color.type, 1, gl_state.array_pointers.color.stride, gl_state.array_pointers.color.pointer);
        real_glEnableVertexAttribArray()(a_color_handle);
    } else {
        real_glVertexAttrib4f()(a_color_handle, gl_state.color.red, gl_state.color.green, gl_state.color.blue, gl_state.color.alpha);
    }

    // Fog
    GLint u_fog_handle = real_glGetUniformLocation()(program, "u_fog");
    real_glUniform1i()(u_fog_handle, gl_state.fog.enabled);
    if (gl_state.fog.enabled) {
        GLint u_fog_color_handle = real_glGetUniformLocation()(program, "u_fog_color");
        real_glUniform4f()(u_fog_color_handle, gl_state.fog.color[0], gl_state.fog.color[1], gl_state.fog.color[2], gl_state.fog.color[3]);
        GLint u_fog_is_linear_handle = real_glGetUniformLocation()(program, "u_fog_is_linear");
        real_glUniform1i()(u_fog_is_linear_handle, gl_state.fog.mode == GL_LINEAR);
        GLint u_fog_start_handle = real_glGetUniformLocation()(program, "u_fog_start");
        real_glUniform1f()(u_fog_start_handle, gl_state.fog.start);
        GLint u_fog_end_handle = real_glGetUniformLocation()(program, "u_fog_end");
        real_glUniform1f()(u_fog_end_handle, gl_state.fog.end);
    }

    // Vertices
    GLint a_vertex_coords_handle = real_glGetAttribLocation()(program, "a_vertex_coords");
    real_glVertexAttribPointer()(a_vertex_coords_handle, gl_state.array_pointers.vertex.size, gl_state.array_pointers.vertex.type, 0, gl_state.array_pointers.vertex.stride, gl_state.array_pointers.vertex.pointer);
    real_glEnableVertexAttribArray()(a_vertex_coords_handle);

    // Texture Coordinates
    GLint a_texture_coords_handle = real_glGetAttribLocation()(program, "a_texture_coords");
    if (use_texture) {
        real_glVertexAttribPointer()(a_texture_coords_handle, gl_state.array_pointers.tex_coord.size, gl_state.array_pointers.tex_coord.type, 0, gl_state.array_pointers.tex_coord.stride, gl_state.array_pointers.tex_coord.pointer);
        real_glEnableVertexAttribArray()(a_texture_coords_handle);
    } else {
        real_glVertexAttrib3f()(a_texture_coords_handle, 0, 0, 0);
    }

    // Draw
    real_glDrawArrays()(mode, first, count);

    // Cleanup
    if (use_color_pointer) {
        real_glDisableVertexAttribArray()(a_color_handle);
    }
    real_glDisableVertexAttribArray()(a_vertex_coords_handle);
    if (use_texture) {
        real_glDisableVertexAttribArray()(a_texture_coords_handle);
    }
}
