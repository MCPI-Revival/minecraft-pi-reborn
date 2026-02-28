project(media-layer)

# Add Core
if(BUILD_MEDIA_LAYER_CORE)
    add_subdirectory(core)
endif()

# Add Trampoline
if(MCPI_USE_MEDIA_LAYER_TRAMPOLINE)
    add_subdirectory(trampoline)
endif()

# Add Headers
if(TARGET media-layer-core)
    setup_header_dirs(media-layer-core "${CMAKE_CURRENT_SOURCE_DIR}/include")
endif()