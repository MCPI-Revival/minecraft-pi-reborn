#include <unistd.h>

#include <libreborn/log.h>

// Debug Tag
const char *reborn_debug_tag = "";

// Debug FD
int reborn_get_debug_fd() {
    if (getenv("MCPI_DEBUG") != NULL) {
        return STDERR_FILENO;
    } else {
        static int debug_fd = -1;
        if (debug_fd == -1) {
            const char *log_file_fd_env = getenv("MCPI_LOG_FILE_FD");
            if (log_file_fd_env == NULL) {
                return -1;
            }
            debug_fd = atoi(log_file_fd_env);
        }
        return debug_fd;
    }
}
