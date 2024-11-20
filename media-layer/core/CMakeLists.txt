project(media-layer-core)

# Build
add_library(media-layer-core-real SHARED
    src/base.cpp
    src/window/media.cpp
    src/window/cursor.cpp
    src/window/util.cpp
    src/window/events.cpp
    src/window/offscreen.cpp
    src/window/gles.cpp
    src/audio/api.cpp
    src/audio/engine.cpp
    src/audio/file.cpp
)

# Set Name
set_target_properties(media-layer-core-real PROPERTIES OUTPUT_NAME "media-layer-core")
if(BUILD_NATIVE_COMPONENTS)
    add_library(media-layer-core ALIAS media-layer-core-real)
endif()

# Install
setup_library(media-layer-core-real TRUE FALSE)

# Link
find_library(OPENAL_LIBRARY NAMES openal REQUIRED)
target_link_libraries(media-layer-core-real
    PUBLIC
        media-layer-headers
        reborn-util
        dl
    PRIVATE
        "${OPENAL_LIBRARY}"
        m
        glfw
        LIB_LIEF
)
