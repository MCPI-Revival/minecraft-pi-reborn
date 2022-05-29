#include <GLES/gl.h>

#include "matrix.h"

// Matrix Data
#define MATRIX_STACK_DEPTH 256
typedef struct {
    matrix_t stack[MATRIX_STACK_DEPTH];
    unsigned int i;
} matrix_stack_t;

// Array Pointer Storage
typedef struct {
    GLboolean enabled;
    GLint size;
    GLenum type;
    GLsizei stride;
    const void *pointer;
} array_pointer_t;

// GL State
typedef struct {
    struct {
        GLfloat red;
        GLfloat green;
        GLfloat blue;
        GLfloat alpha;
    } color;
    struct {
        GLenum mode;
        matrix_stack_t model_view;
        matrix_stack_t projection;
        matrix_stack_t texture;
    } matrix_stacks;
    struct {
        array_pointer_t vertex;
        array_pointer_t color;
        array_pointer_t tex_coord;
    } array_pointers;
    GLboolean alpha_test;
    GLboolean texture_2d;
    struct {
        GLboolean enabled;
        GLfixed mode;
        GLfloat color[4];
        GLfloat start;
        GLfloat end;
    } fog;
} gl_state_t;
extern gl_state_t gl_state;
