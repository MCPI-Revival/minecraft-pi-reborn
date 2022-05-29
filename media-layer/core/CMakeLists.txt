project(media-layer-core)

# Configuration
set(CORE_SRC src/base.cpp src/media.c src/screenshot.c) # SDL Re-Implementation Using GLFW
if(NOT MCPI_HEADLESS_MODE)
    list(APPEND CORE_SRC src/audio/api.cpp src/audio/engine.c src/audio/file.cpp)
endif()

# Build
if(MCPI_USE_MEDIA_LAYER_PROXY AND BUILD_NATIVE_COMPONENTS)
    # Build Media Layer Core Natively And Use Proxy
    add_library(media-layer-core-real OBJECT ${CORE_SRC}) # Dependencies Are Setup Later
endif()
if(NOT MCPI_USE_MEDIA_LAYER_PROXY AND BUILD_ARM_COMPONENTS)
    # Directly Link Media Layer Core To MCPI
    add_library(media-layer-core-real SHARED ${CORE_SRC}) # Dependencies Are Setup Later
    set_target_properties(media-layer-core-real PROPERTIES OUTPUT_NAME "media-layer-core")
    # Install
    install(TARGETS media-layer-core-real DESTINATION "${MCPI_LIB_DIR}")
    # Create Alias Target For Linking
    add_library(media-layer-core ALIAS media-layer-core-real)
endif()

# Configure Media Layer Core If Built
if(TARGET media-layer-core-real)
    # Link
    target_link_libraries(media-layer-core-real media-layer-headers reborn-util pthread dl)
    if(NOT MCPI_HEADLESS_MODE)
        # Find FreeImage
        find_library(FREEIMAGE_LIBRARY NAMES freeimage libfreeimage.so.3 REQUIRED)
        # OpenAL
        find_library(OPENAL_LIBRARY NAMES openal REQUIRED)
        # Link
        target_link_libraries(media-layer-core-real "${FREEIMAGE_LIBRARY}" "${OPENAL_LIBRARY}" m GLESv1_CM glfw)
    endif()
endif()

# Add Symlinks So MCPI Can Locate Libraries
if(BUILD_ARM_COMPONENTS)
    install_symlink("libmedia-layer-core.so" "${MCPI_LIB_DIR}/libSDL-1.2.so.0")
endif()
