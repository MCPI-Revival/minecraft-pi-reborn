project(media-layer-extras)

if(BUILD_ARM_COMPONENTS)
    # Add Source To Media Core
    if(TARGET media-layer-core-real)
        set(TARGET media-layer-core-real)
    elseif(TARGET media-layer-proxy-server)
        set(TARGET media-layer-proxy-server)
    endif()
    target_sources("${TARGET}" PRIVATE src/SDL.c)
endif()
