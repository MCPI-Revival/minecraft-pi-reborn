#include <GLES/gl.h>

// Matrix Common
#define MATRIX_SIZE 4
#define MATRIX_DATA_SIZE (sizeof (GLfloat) * MATRIX_SIZE * MATRIX_SIZE)
// OpenGL Matrices Are Column-Major
typedef struct {
    GLfloat data[MATRIX_SIZE][MATRIX_SIZE];
} matrix_t;
