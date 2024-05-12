#include <vector>

#include <SDL/SDL.h>

#include <media-layer/internal.h>
#include <media-layer/core.h>
#include <libreborn/libreborn.h>

// SDL Is Replaced With GLFW

int SDL_Init(__attribute__((unused)) uint32_t flags) {
    return 0;
}

// Event Queue

static std::vector<SDL_Event> queue;

int SDL_PollEvent(SDL_Event *event) {
    // Handle External Events
    _media_handle_SDL_PollEvent();

    // Poll Event
    int ret;
    if (queue.size() > 0) {
        *event = queue[0];
        queue.erase(queue.begin());
        ret = 1;
    } else {
        ret = 0;
    }
    return ret;
}

int SDL_PushEvent(SDL_Event *event) {
    queue.push_back(*event);
    return 1;
}

void media_ensure_loaded() {
    // NOP
}
