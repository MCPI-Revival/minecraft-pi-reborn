project(media-layer-core)

# Dependencies
add_subdirectory(dependencies)

# Configuration
set(CORE_SRC src/base.cpp src/media.c src/screenshot.c $<TARGET_OBJECTS:media-layer-extras>) # SDL Re-Implementation Using GLFW
if(NOT MCPI_HEADLESS_MODE)
    list(APPEND CORE_SRC src/audio/api.cpp src/audio/engine.c src/audio/file.cpp)
endif()

# Build
add_library(media-layer-core SHARED ${CORE_SRC}) # Dependencies Are Setup Later
# Install
install(TARGETS media-layer-core DESTINATION "${MCPI_LIB_DIR}")
install(TARGETS media-layer-core EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")

# Link
target_link_libraries(media-layer-core PUBLIC media-layer-headers PUBLIC reborn-util PUBLIC pthread PUBLIC dl)
if(NOT MCPI_HEADLESS_MODE)
    # OpenAL
    find_library(OPENAL_LIBRARY NAMES openal REQUIRED)
    # Link
    target_link_libraries(media-layer-core PRIVATE png12 PRIVATE "${OPENAL_LIBRARY}" PRIVATE m PUBLIC GLESv1_CM PRIVATE glfw)
endif()
