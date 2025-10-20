project(media-layer-core)

# Build
add_library(media-layer-core SHARED
    src/base.cpp
    src/window/media.cpp
    src/window/cursor.cpp
    src/window/util.cpp
    src/window/events.cpp
    src/window/offscreen.cpp
    src/window/opengl/api.cpp
    src/window/opengl/texture.cpp
    src/audio/api.cpp
    src/audio/engine.cpp
    src/audio/file.cpp
)

# Install
setup_library(media-layer-core TRUE TRUE)

# Link
find_library(OPENAL_LIBRARY NAMES openal REQUIRED)
target_link_libraries(media-layer-core
    PUBLIC
        media-layer-headers
        reborn-util
        dl
    PRIVATE
        "${OPENAL_LIBRARY}"
        m
        glfw
        LIB_LIEF
        stb_image
)
