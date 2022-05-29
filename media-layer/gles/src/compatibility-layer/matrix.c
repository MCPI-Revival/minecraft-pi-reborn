#include <math.h>
#include <string.h>

#include <libreborn/libreborn.h>

#include "state.h"
#include "passthrough.h"

// Matrix Common
static void matrix_copy(matrix_t *src, matrix_t *dst) {
    memcpy((void *) dst->data, (void *) src->data, MATRIX_DATA_SIZE);
}

// Identity Matrix
static matrix_t identity_matrix = {
    .data = {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    }
};
static void init_matrix_stack(matrix_stack_t *stack) {
    matrix_copy(&identity_matrix, &stack->stack[0]);
}
__attribute__((constructor)) static void init_matrix_stacks() {
    init_matrix_stack(&gl_state.matrix_stacks.model_view);
    init_matrix_stack(&gl_state.matrix_stacks.projection);
    init_matrix_stack(&gl_state.matrix_stacks.texture);
}

// Matrix Mode
static matrix_stack_t *get_matrix_stack() {
    switch (gl_state.matrix_stacks.mode) {
        case GL_MODELVIEW: {
            return &gl_state.matrix_stacks.model_view;
        }
        case GL_PROJECTION: {
            return &gl_state.matrix_stacks.projection;
        }
        case GL_TEXTURE: {
            return &gl_state.matrix_stacks.texture;
        }
        default: {
            ERR("Unsupported Matrix Mode: %i", gl_state.matrix_stacks.mode);
        }
    }
}

// Matrix Functions
void glMatrixMode(GLenum mode) {
    gl_state.matrix_stacks.mode = mode;
}
void glPopMatrix() {
    get_matrix_stack()->i--;
}
void glLoadIdentity() {
    matrix_stack_t *stack = get_matrix_stack();
    matrix_copy(&identity_matrix, &stack->stack[stack->i]);
}
void glPushMatrix() {
    matrix_stack_t *stack = get_matrix_stack();
    matrix_copy(&stack->stack[stack->i], &stack->stack[stack->i + 1]);
    stack->i++;
}
void glMultMatrixf(const GLfloat *m) {
    matrix_t new_matrix;
    matrix_stack_t *stack = get_matrix_stack();
    matrix_t *current_matrix = &stack->stack[stack->i];
    for (int x = 0; x < MATRIX_SIZE; x++) {
        for (int y = 0; y < MATRIX_SIZE; y++) {
            GLfloat result = 0;
            for (int i = 0; i < MATRIX_SIZE; i++) {
                result += (current_matrix->data[i][y] * m[(x * MATRIX_SIZE) + i]);
            }
            new_matrix.data[x][y] = result;
        }
    }
    matrix_copy(&new_matrix, current_matrix);
}
void glScalef(GLfloat x, GLfloat y, GLfloat z) {
    GLfloat m[] = {
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1
    };
    glMultMatrixf(m);
}
void glTranslatef(GLfloat x, GLfloat y, GLfloat z) {
    GLfloat m[] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, z, 1
    };
    glMultMatrixf(m);
}
void glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far) {
    GLfloat m[] = {
        (2.f / (right - left)), 0, 0, 0,
        0, (2.f / (top - bottom)), 0, 0,
        0, 0, (-2.f / (far - near)), 0,
        -((right + left) / (right - left)), -((top + bottom) / (top - bottom)), -((far + near) / (far - near)), 1
    };
    glMultMatrixf(m);
}
#define DEG2RAD (M_PI / 180.f)
void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
    // Normalize
    GLfloat length = sqrtf((x * x) + (y * y) + (z * z));
    x /= length;
    y /= length;
    z /= length;

    // Values
    GLfloat angle_radians = angle * DEG2RAD;
    GLfloat c = cosf(angle_radians);
    GLfloat s = sinf(angle_radians);
    GLfloat x2 = x * x;
    GLfloat y2 = y * y;
    GLfloat z2 = z * z;

    // Multiply
    GLfloat m[] = {
        x2 * (1.f - c) + c, (x * y) * (1.f - c) + (z * s), (x * z) * (1.f - c) - (y * s), 0,
        (x * y) * (1.f - c) - (z * s), y2 * (1.f - c) + c, (y * z) * (1.f - c) + (x * s), 0,
        (x * z) * (1.f - c) + (y * s), (y * z) * (1.f - c) - (x * s), z2 * (1.f - c) + c, 0,
        0, 0, 0, 1.f
    };
    glMultMatrixf(m);
}
