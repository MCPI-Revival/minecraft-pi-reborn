#include <GLES/gl.h>

// Matrix Common
#define MATRIX_SIZE 4
#define MATRIX_DATA_SIZE (sizeof (float) * MATRIX_SIZE * MATRIX_SIZE)
// OpenGL Matricies Are Column-Major
typedef struct {
    GLfloat data[MATRIX_SIZE][MATRIX_SIZE];
} matrix_t;
