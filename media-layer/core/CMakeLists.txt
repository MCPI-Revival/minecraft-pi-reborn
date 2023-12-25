project(media-layer-core)

# OpenGL
if(NOT MCPI_HEADLESS_MODE)
    add_subdirectory(gles)
endif()

# Configuration
set(CORE_SRC src/base.cpp src/media.c $<TARGET_OBJECTS:media-layer-extras>) # SDL Re-Implementation Using GLFW
if(NOT MCPI_HEADLESS_MODE)
    list(APPEND CORE_SRC src/audio/api.cpp src/audio/engine.c src/audio/file.cpp)
else()
    list(APPEND CORE_SRC src/audio/stubs.c)
endif()

# Build
add_library(media-layer-core-real SHARED ${CORE_SRC}) # Dependencies Are Setup Later
set_target_properties(media-layer-core-real PROPERTIES OUTPUT_NAME "media-layer-core")
if(BUILD_NATIVE_COMPONENTS)
    add_library(media-layer-core ALIAS media-layer-core-real)
endif()
# Install
install(TARGETS media-layer-core-real DESTINATION "${MCPI_LIB_DIR}")

# Link
target_link_libraries(media-layer-core-real PUBLIC media-layer-headers PUBLIC reborn-util PUBLIC dl)
if(NOT MCPI_HEADLESS_MODE)
    # OpenAL
    find_library(OPENAL_LIBRARY NAMES openal REQUIRED)
    # Link
    target_link_libraries(media-layer-core-real PRIVATE "${OPENAL_LIBRARY}" PRIVATE m PRIVATE glfw PUBLIC GLESv1_CM PRIVATE LIB_LIEF)
endif()
