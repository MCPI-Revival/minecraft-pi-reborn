project(media-layer-core)

# Build
add_library(media-layer-core SHARED
    src/base.cpp
    src/window/media.cpp
    src/window/cursor.cpp
    src/window/util.cpp
    src/window/events.cpp
    src/window/offscreen.cpp
    src/window/opengl/api/passthrough.cpp
    src/window/opengl/texture.cpp
    src/audio/api.cpp
    src/audio/engine.cpp
    src/audio/file.cpp
)

# Install
setup_library(media-layer-core TRUE TRUE)

# Link
find_package(OpenAL REQUIRED)
target_link_libraries(media-layer-core
    PUBLIC
        media-layer-headers
        reborn-util
    PRIVATE
        OpenAL::OpenAL
        m
        glfw
        LIB_LIEF
        stb_image
)
if(NOT MCPI_WIN32)
    target_link_libraries(media-layer-core PUBLIC
        dl
    )
endif()
