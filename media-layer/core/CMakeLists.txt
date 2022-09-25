project(media-layer-core)

# Dependencies
add_subdirectory(dependencies)

# OpenGL
add_subdirectory(gles)

# Configuration
set(CORE_SRC src/base.cpp src/media.c $<TARGET_OBJECTS:media-layer-extras>) # SDL Re-Implementation Using GLFW
if(NOT MCPI_HEADLESS_MODE)
    list(APPEND CORE_SRC src/audio/api.cpp src/audio/engine.c src/audio/file.cpp)
else()
    list(APPEND CORE_SRC src/audio/stubs.c)
endif()

# Build
add_library(media-layer-core SHARED ${CORE_SRC}) # Dependencies Are Setup Later
# Install
install(TARGETS media-layer-core DESTINATION "${MCPI_LIB_DIR}")
if(BUILD_ARM_COMPONENTS)
    install(TARGETS media-layer-core EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")
endif()

# Link
target_link_libraries(media-layer-core PUBLIC media-layer-headers PUBLIC reborn-util PUBLIC GLESv1_CM PUBLIC dl)
if(NOT MCPI_HEADLESS_MODE)
    # OpenAL
    find_library(OPENAL_LIBRARY NAMES openal REQUIRED)
    # Link
    target_link_libraries(media-layer-core PRIVATE "${OPENAL_LIBRARY}" PRIVATE m PRIVATE glfw PRIVATE LIB_LIEF)
endif()
