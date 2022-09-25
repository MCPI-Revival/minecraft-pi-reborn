#include <fcntl.h>
#include <sys/file.h>

#include <libreborn/util.h>

// Safe Version Of pipe()
void safe_pipe2(int pipefd[2], int flags) {
    if (pipe2(pipefd, flags) != 0) {
        ERR("Unable To Create Pipe: %s", strerror(errno));
    }
}

// Check If Two Percentages Are Different Enough To Be Logged
#define SIGNIFICANT_PROGRESS 5
int is_progress_difference_significant(int32_t new_val, int32_t old_val) {
    if (new_val != old_val) {
        if (new_val == -1 || old_val == -1) {
            return 1;
        } else if (new_val == 0 || new_val == 100) {
            return 1;
        } else {
            return new_val - old_val >= SIGNIFICANT_PROGRESS;
        }
    } else {
        return 0;
    }
}

// Lock File
int lock_file(const char *file) {
    int fd = open(file, O_WRONLY | O_CREAT, S_IWUSR);
    if (fd == -1) {
        ERR("Unable To Open Lock File: %s: %s", file, strerror(errno));
    }
    if (flock(fd, LOCK_EX) == -1) {
        ERR("Unable To Lock File: %s: %s", file, strerror(errno));
    }
    return fd;
}
void unlock_file(const char *file, int fd) {
    if (flock(fd, LOCK_UN) == -1) {
        ERR("Unable To Unlock File: %s: %s", file, strerror(errno));
    }
    close(fd);
}
