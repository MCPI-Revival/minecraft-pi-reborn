project(media-layer-proxy)

# Configuration
set(MEDIA_LAYER_PROXY_SRC src/common/common.c src/media-layer-core.c src/GLESv1_CM.c) # Media Layer Proxy Source
set(MEDIA_LAYER_PROXY_LIBS media-layer-core GLESv1_CM) # Media Layer Proxy Client Dependencies

if(BUILD_NATIVE_COMPONENTS)
    # Building Native Components

    # Build Media Layer Proxy Client
    add_executable(media-layer-proxy-client src/client/client.cpp ${MEDIA_LAYER_PROXY_SRC})
    target_link_libraries(media-layer-proxy-client media-layer-headers reborn-util ${MEDIA_LAYER_PROXY_LIBS})
    target_compile_definitions(media-layer-proxy-client PRIVATE -DMEDIA_LAYER_PROXY_CLIENT)
    # Install
    install(TARGETS media-layer-proxy-client DESTINATION "${MCPI_BIN_DIR}")
endif()

if(BUILD_ARM_COMPONENTS)
    # Building ARM Components

    # Build Media Layer Proxy Server
    add_library(media-layer-core SHARED src/server/server.cpp ${MEDIA_LAYER_PROXY_SRC})
    target_link_libraries(media-layer-core media-layer-headers reborn-util)
    target_compile_definitions(media-layer-core PRIVATE -DMEDIA_LAYER_PROXY_SERVER)
    # Symlink GLESv1_CM To Media Layer Proxy Server
    install_symlink("libmedia-layer-core.so" "${MCPI_LIB_DIR}/libGLESv1_CM.so.1")
    # Install
    install(TARGETS media-layer-core DESTINATION "${MCPI_LIB_DIR}")
endif()
