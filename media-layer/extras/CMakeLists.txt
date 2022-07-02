project(media-layer-extras)

# Build
add_library(media-layer-extras OBJECT src/SDL.c)
target_link_libraries(media-layer-extras media-layer-headers reborn-util)
