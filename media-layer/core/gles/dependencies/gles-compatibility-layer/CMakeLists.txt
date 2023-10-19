project(media-layer-core-gles-dependencies-gles-compatibility-layer)

# GLES Compatibility Layer
set(GLES_COMPATIBILITY_LAYER_USE_DEFAULT_INCLUDE_PATH FALSE CACHE BOOL "" FORCE)
set(GLES_COMPATIBILITY_LAYER_USE_ES3 FALSE CACHE BOOL "" FORCE)
set(GLES_COMPATIBILITY_LAYER_USE_SDL FALSE CACHE BOOL "" FORCE)
set(GLES_COMPATIBILITY_LAYER_DEPENDENCY glfw CACHE STRING "" FORCE)
add_subdirectory(src)
target_link_libraries(gles-compatibility-layer media-layer-headers)
