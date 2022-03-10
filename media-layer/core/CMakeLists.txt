project(media-layer-core)

# Configuration
set(CORE_SRC src/base.cpp src/media.c src/screenshot.c) # SDL Re-Implementation Using GLFW
if(NOT MCPI_HEADLESS_MODE)
    list(APPEND CORE_SRC src/audio/api.cpp src/audio/engine.c src/audio/file.cpp)
endif()

# Build
if(MCPI_USE_MEDIA_LAYER_PROXY AND BUILD_NATIVE_COMPONENTS)
    # Building Native Components
    add_library(media-layer-core OBJECT ${CORE_SRC}) # Dependencies Are Setup Later
elseif(NOT MCPI_USE_MEDIA_LAYER_PROXY AND BUILD_ARM_COMPONENTS)
    # Building ARM Components
    add_library(media-layer-core SHARED ${CORE_SRC}) # Dependencies Are Setup Later
    # Install
    install(TARGETS media-layer-core DESTINATION "${MCPI_LIB_DIR}")
endif()

# Configure Media Layer Core If Built
if(TARGET media-layer-core)
    # Link
    target_link_libraries(media-layer-core media-layer-headers reborn-headers pthread dl)
    if(NOT MCPI_HEADLESS_MODE)
        # Find FreeImage
        find_library(FREEIMAGE_LIBRARY NAMES freeimage libfreeimage.so.3 REQUIRED)
        # OpenAL
        find_library(OPENAL_LIBRARY NAMES openal REQUIRED)
        # Link
        target_link_libraries(media-layer-core "${FREEIMAGE_LIBRARY}" "${OPENAL_LIBRARY}" m GLESv1_CM glfw)
    endif()
endif()

# Add Symlinks So MCPI Can Locate Libraries
if(BUILD_ARM_COMPONENTS)
    install_symlink("libmedia-layer-core.so" "${MCPI_LIB_DIR}/libSDL-1.2.so.0")
endif()
