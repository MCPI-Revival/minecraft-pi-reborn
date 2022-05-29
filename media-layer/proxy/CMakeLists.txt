project(media-layer-proxy)

# Configuration
set(MEDIA_LAYER_PROXY_SRC src/common/common.c src/media-layer-core.c src/GLESv1_CM.c) # Media Layer Proxy Source

# Build
if(BUILD_NATIVE_COMPONENTS)
    # Build Media Layer Proxy Client
    add_executable(media-layer-proxy-client src/client/client.cpp ${MEDIA_LAYER_PROXY_SRC})
    target_link_libraries(media-layer-proxy-client media-layer-headers reborn-util media-layer-core-real GLESv1_CM)
    target_compile_definitions(media-layer-proxy-client PRIVATE -DMEDIA_LAYER_PROXY_CLIENT)
    # Install
    install(TARGETS media-layer-proxy-client DESTINATION "${MCPI_BIN_DIR}")
endif()
if(BUILD_ARM_COMPONENTS)
    # Build Media Layer Proxy Server
    add_library(media-layer-proxy-server SHARED src/server/server.cpp ${MEDIA_LAYER_PROXY_SRC})
    target_link_libraries(media-layer-proxy-server media-layer-headers reborn-util)
    target_compile_definitions(media-layer-proxy-server PRIVATE -DMEDIA_LAYER_PROXY_SERVER)
    set_target_properties(media-layer-proxy-server PROPERTIES OUTPUT_NAME "media-layer-core")
    # Symlink GLESv1_CM To Media Layer Proxy Server
    install_symlink("libmedia-layer-core.so" "${MCPI_LIB_DIR}/libGLESv1_CM.so.1")
    # Install
    install(TARGETS media-layer-proxy-server DESTINATION "${MCPI_LIB_DIR}")
    # Create Alias Target For Linking
    add_library(media-layer-core ALIAS media-layer-proxy-server)
endif()
