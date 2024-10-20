#include <vector>

#include <SDL/SDL.h>

#include <media-layer/core.h>
#include <libreborn/libreborn.h>

#include "media.h"

// SDL Is Replaced With GLFW

int media_SDL_Init(__attribute__((unused)) uint32_t flags) {
    return 0;
}

// Event Queue

static std::vector<SDL_Event> queue;

int media_SDL_PollEvent(SDL_Event *event) {
    // Handle External Events
    _media_handle_media_SDL_PollEvent();

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

int media_SDL_PushEvent(SDL_Event *event) {
    queue.push_back(*event);
    return 1;
}

void media_ensure_loaded() {
    // NOP
}
