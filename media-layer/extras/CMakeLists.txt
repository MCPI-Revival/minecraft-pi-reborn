project(media-layer-extras)

if(BUILD_ARM_COMPONENTS)
    # Add Source To Media Core
    target_sources(media-layer-core PRIVATE src/SDL.c src/X11.cpp)
    target_link_libraries(media-layer-core dl)
endif()
