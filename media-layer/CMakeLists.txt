project(media-layer)

# Add Headers
add_library(media-layer-headers INTERFACE)
setup_header_dirs(media-layer-headers "${CMAKE_CURRENT_SOURCE_DIR}/include")
setup_library(media-layer-headers FALSE TRUE)

# Add Core
if(BUILD_MEDIA_LAYER_CORE)
    add_subdirectory(core)
endif()

# Add Trampoline
if(MCPI_USE_MEDIA_LAYER_TRAMPOLINE OR BUILD_ARM_COMPONENTS)
    add_subdirectory(trampoline)
endif()
