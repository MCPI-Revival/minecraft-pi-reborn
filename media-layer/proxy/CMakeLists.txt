project(media-layer-proxy)

# Configuration
set(MEDIA_LAYER_PROXY_SRC src/common/common.c src/media-layer-core.c src/GLESv1_CM.c) # Media Layer Proxy Source

# Build
if(BUILD_NATIVE_COMPONENTS)
    # Build Media Layer Proxy Client
    add_executable(media-layer-proxy-client src/client/client.cpp ${MEDIA_LAYER_PROXY_SRC})
    target_link_libraries(media-layer-proxy-client media-layer-headers reborn-util media-layer-core GLESv1_CM)
    target_compile_definitions(media-layer-proxy-client PRIVATE -DMEDIA_LAYER_PROXY_CLIENT)
    # Install
    install(TARGETS media-layer-proxy-client DESTINATION "${MCPI_BIN_DIR}")
elseif(BUILD_ARM_COMPONENTS)
    # Build Media Layer Proxy Server
    add_library(media-layer-core SHARED src/server/server.cpp ${MEDIA_LAYER_PROXY_SRC} $<TARGET_OBJECTS:media-layer-extras>)
    target_link_libraries(media-layer-core media-layer-headers reborn-util)
    target_compile_definitions(media-layer-core PRIVATE -DMEDIA_LAYER_PROXY_SERVER)
    # Install
    if(MCPI_USE_MEDIA_LAYER_PROXY)
        install(TARGETS media-layer-core DESTINATION "${MCPI_LIB_DIR}")
    endif()
    install(TARGETS media-layer-core EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")
endif()
