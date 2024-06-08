project(media-layer-trampoline)

# Configuration
set(MEDIA_LAYER_TRAMPOLINE_SRC src/media-layer-core.cpp) # Media Layer Trampoline Source
if(NOT MCPI_HEADLESS_MODE)
    list(APPEND MEDIA_LAYER_TRAMPOLINE_SRC src/GLESv1_CM.cpp)
endif()

# Build
if(BUILD_NATIVE_COMPONENTS)
    # Host Component
    add_library(media-layer-trampoline src/host/host.cpp ${MEDIA_LAYER_TRAMPOLINE_SRC})
    target_link_libraries(media-layer-trampoline reborn-util media-layer-core trampoline-headers)
    if(NOT MCPI_HEADLESS_MODE)
        target_link_libraries(media-layer-trampoline GLESv1_CM)
    endif()
    target_compile_definitions(media-layer-trampoline PRIVATE -DMEDIA_LAYER_TRAMPOLINE_HOST)
    # Install
    install(TARGETS media-layer-trampoline DESTINATION "${MCPI_LIB_DIR}")
elseif(BUILD_ARM_COMPONENTS)
    # Guest Component
    add_library(media-layer-core SHARED src/guest/guest.cpp ${MEDIA_LAYER_TRAMPOLINE_SRC} $<TARGET_OBJECTS:media-layer-extras>)
    target_link_libraries(media-layer-core PUBLIC media-layer-headers PRIVATE reborn-util PRIVATE trampoline-headers PRIVATE rt)
    target_compile_definitions(media-layer-core PRIVATE -DMEDIA_LAYER_TRAMPOLINE_GUEST)
    # Install
    if(MCPI_USE_MEDIA_LAYER_TRAMPOLINE)
        install(TARGETS media-layer-core DESTINATION "${MCPI_LIB_DIR}")
    endif()
    install(TARGETS media-layer-core EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")
endif()
