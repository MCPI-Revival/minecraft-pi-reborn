project(media-layer)

# Check Options
if(MCPI_USE_MEDIA_LAYER_PROXY AND MCPI_BUILD_MODE STREQUAL "both")
    message(FATAL_ERROR "Media Layer Proxy Is Redundant When Building ARM And Native Components In The Same Build")
endif()

# Add Headers
add_library(media-layer-headers INTERFACE)
target_include_directories(media-layer-headers INTERFACE include)

# Add Core
add_subdirectory(core)

# Add Proxy
if(MCPI_USE_MEDIA_LAYER_PROXY)
    add_subdirectory(proxy)
endif()

# Add Stubs
add_subdirectory(stubs)

# Add Extras
add_subdirectory(extras)
