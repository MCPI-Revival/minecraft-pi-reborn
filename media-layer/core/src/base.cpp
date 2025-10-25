#include <queue>

#include <libreborn/util/exec.h>
#include <SDL/SDL.h>

#include "window/media.h"

// SDL Is Replaced With GLFW
int media_SDL_Init(MCPI_UNUSED uint32_t flags) {
    return 0;
}

// Event Queue
static std::queue<SDL_Event> queue;
int media_SDL_PollEvent(SDL_Event *event) {
    // Handle External Events
    _media_handle_media_SDL_PollEvent();

    // Poll Event
    int ret;
    if (!queue.empty()) {
        *event = queue.front();
        queue.pop();
        ret = 1;
    } else {
        ret = 0;
    }
    return ret;
}
int media_SDL_PushEvent(const SDL_Event *event) {
    queue.push(*event);
    return 1;
}

// Open File Or URL
void media_open(const char *path, const bool is_url) {
    // Convert To URL
    std::string url = path;
    if (!is_url) {
#ifdef _WIN32
        const char *const command[] = {
            "wsl",
            WSL_FLAGS,
            "--exec", "wslpath", "-w", url.c_str(),
            nullptr
        };
        int exit_code;
        const std::vector<unsigned char> *output = run_command(command, &exit_code);
        if (!is_exit_status_success(exit_code)) {
            // Give Up
            return;
        }
        url = (const char *) output->data();
        delete output;
        trim(url);
#else
        url = "file://" + url;
#endif
    }
    // Open
    open_url(url);
}