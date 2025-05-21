#include <queue>

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
