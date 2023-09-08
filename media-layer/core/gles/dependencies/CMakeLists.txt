project(media-layer-core-gles-dependencies)

# GLES Compatibility Layer
if(NOT MCPI_HEADLESS_MODE AND MCPI_USE_GLES1_COMPATIBILITY_LAYER)
    set(GLES_COMPATIBILITY_LAYER_USE_DEFAULT_INCLUDE_PATH FALSE CACHE BOOL "" FORCE)
    set(GLES_COMPATIBILITY_LAYER_USE_ES3 FALSE CACHE BOOL "" FORCE)
    set(GLES_COMPATIBILITY_LAYER_USE_SDL FALSE CACHE BOOL "" FORCE)
    set(GLES_COMPATIBILITY_LAYER_DEPENDENCY glfw CACHE STRING "" FORCE)
    add_subdirectory(gles-compatibility-layer)
    target_link_libraries(gles-compatibility-layer media-layer-headers)
endif()
