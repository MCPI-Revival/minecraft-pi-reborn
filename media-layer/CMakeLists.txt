project(media-layer)

# Add Headers
add_library(media-layer-headers INTERFACE)
target_include_directories(
    media-layer-headers
    INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${MCPI_SDK_INCLUDE_DIR}/media-layer>"
)
# SDK
if(BUILD_ARM_COMPONENTS)
    install(TARGETS media-layer-headers EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")
    install(DIRECTORY "include/" DESTINATION "${MCPI_SDK_INCLUDE_DIR}/media-layer")
endif()

# Add Extras
add_subdirectory(extras)

# Add Core
if((BUILD_NATIVE_COMPONENTS AND MCPI_USE_MEDIA_LAYER_PROXY) OR (BUILD_ARM_COMPONENTS AND NOT MCPI_USE_MEDIA_LAYER_PROXY))
    add_subdirectory(core)
endif()

# Add Proxy
if(MCPI_USE_MEDIA_LAYER_PROXY)
    add_subdirectory(proxy)
endif()
