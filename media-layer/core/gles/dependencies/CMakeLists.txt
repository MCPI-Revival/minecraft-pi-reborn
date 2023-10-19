project(media-layer-core-gles-dependencies)

# GLES Compatibility Layer
if(NOT MCPI_HEADLESS_MODE AND MCPI_USE_GLES1_COMPATIBILITY_LAYER)
    add_subdirectory(gles-compatibility-layer)
endif()
