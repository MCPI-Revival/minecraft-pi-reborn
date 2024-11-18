project(media-layer-core)

# SDL Re-Implementation Using GLFW
set(CORE_SRC
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

# Build
add_library(media-layer-core-real SHARED ${CORE_SRC}) # Dependencies Are Setup Later
set_target_properties(media-layer-core-real PROPERTIES OUTPUT_NAME "media-layer-core")
if(BUILD_NATIVE_COMPONENTS)
    add_library(media-layer-core ALIAS media-layer-core-real)
endif()
# Install
install(TARGETS media-layer-core-real DESTINATION "${MCPI_LIB_DIR}")

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
