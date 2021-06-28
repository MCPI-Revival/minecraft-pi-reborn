project(media-layer)

# Check Options
if(MCPI_USE_MEDIA_LAYER_PROXY)
    if(MCPI_SERVER_MODE)
        message(FATAL_ERROR "Server Mode With Media Layer Proxy Configuration Is Redundant")
    endif()
    if(MCPI_BUILD_MODE STREQUAL "both")
        message(FATAL_ERROR "Media Layer Proxy Is Redundant When Building ARM And Native Components In The Same Build")
    endif()
endif()

# Add Headers
add_library(media-layer-headers INTERFACE)
target_include_directories(media-layer-headers INTERFACE include)

# Add Core
add_subdirectory(core)

# Add Proxy
if(MCPI_USE_MEDIA_LAYER_PROXY)
    add_subdirectory(proxy)
endif()

# Add Stubs
add_subdirectory(stubs)

# Add Extras
add_subdirectory(extras)

# Add Symlinks So MCPI Can Locate Libraries
if(BUILD_ARM_COMPONENTS)
    install_symlink("libmedia-layer-core.so" "${MCPI_LIB_DIR}/libSDL-1.2.so.0")
endif()
