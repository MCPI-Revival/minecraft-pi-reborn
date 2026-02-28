project(media-layer-core)

# Build
add_library(media-layer-core SHARED
    src/base.cpp
    src/window/media.cpp
    src/window/cursor.cpp
    src/window/util.cpp
    src/window/events.cpp
    src/window/offscreen.cpp
    src/window/opengl/api/core.cpp
    src/window/opengl/api/framebuffer.cpp
    src/window/opengl/texture.cpp
    src/audio/api.cpp
    src/audio/engine.cpp
    src/audio/file.cpp
)

# Install
setup_library(media-layer-core
    INSTALL
    IN_SDK
    DEFINE_EXPORT_MACRO
)

# Link
target_link_libraries(media-layer-core
    PUBLIC
        reborn-util
    PRIVATE
        openal
        m
        glfw
        LIB_LIEF
        stb_image
        Threads::Threads
)
if(NOT MCPI_WIN32)
    target_link_libraries(media-layer-core PUBLIC
        dl
    )
endif()
