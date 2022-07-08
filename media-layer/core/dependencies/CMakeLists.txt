project(media-layer-core-dependencies)

# GLFW
if(NOT MCPI_HEADLESS_MODE)
    add_subdirectory(glfw)
endif()
