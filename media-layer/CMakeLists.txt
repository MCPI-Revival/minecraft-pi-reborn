project(media-layer)

# Target Notes:
#     media-layer-core-real: Fully Built Media Layer Core
#     media-layer-core: Alias Target That Points To The Library MCPI Should Link To

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

# Add GLESv1_CM Stubs Or Compatibility Layer
add_subdirectory(gles)

# Add Core
add_subdirectory(core)

# Add Proxy
if(MCPI_USE_MEDIA_LAYER_PROXY)
    add_subdirectory(proxy)
endif()

# Add Extras
add_subdirectory(extras)
