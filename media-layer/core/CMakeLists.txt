project(media-layer-core)

# SDL Re-Implementation Using GLFW
set(CORE_SRC
    src/base.cpp
    src/media.cpp
    src/audio/api.cpp
    src/audio/engine.c
    src/audio/file.cpp
    $<TARGET_OBJECTS:media-layer-extras>
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
    PUBLIC media-layer-headers
    PUBLIC reborn-util
    PRIVATE "${OPENAL_LIBRARY}"
    PRIVATE m
    PRIVATE glfw
    PUBLIC GLESv1_CM
    PRIVATE LIB_LIEF
    PUBLIC dl
)
