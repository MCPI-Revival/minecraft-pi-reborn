project(media-layer-trampoline)

# Common Sources
set(MEDIA_LAYER_TRAMPOLINE_SRC
    src/media-layer-core.cpp
    src/GLESv1_CM.cpp
    src/opengl/state.cpp
)

# Build
if(BUILD_NATIVE_COMPONENTS)
    # Host Component
    add_library(media-layer-trampoline
        src/host/host.cpp
        ${MEDIA_LAYER_TRAMPOLINE_SRC}
    )
    target_link_libraries(media-layer-trampoline
        PRIVATE
            reborn-util
            media-layer-core
            trampoline-headers
    )
    target_compile_definitions(media-layer-trampoline PRIVATE MEDIA_LAYER_TRAMPOLINE_HOST)
    # Install
    setup_library(media-layer-trampoline TRUE TRUE)
elseif(BUILD_ARM_COMPONENTS)
    # Guest Component
    add_library(media-layer-core SHARED
        ${MEDIA_LAYER_TRAMPOLINE_SRC}
    )
    target_link_libraries(media-layer-core
        PUBLIC
            media-layer-headers
        PRIVATE
            reborn-util
            trampoline
    )
    target_compile_definitions(media-layer-core PRIVATE MEDIA_LAYER_TRAMPOLINE_GUEST)
    # Install
    setup_library(media-layer-core TRUE TRUE)
endif()
