#include <libreborn/util.h>

// Safe Version Of pipe()
void safe_pipe2(int pipefd[2], int flags) {
    if (pipe2(pipefd, flags) != 0) {
        ERR("Unable To Create Pipe: %s", strerror(errno));
    }
}
