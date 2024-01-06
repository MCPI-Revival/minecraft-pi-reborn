project(dependencies)

# stb_image
if(BUILD_ARM_COMPONENTS AND NOT MCPI_HEADLESS_MODE)
    add_subdirectory(stb_image)
endif()
# Minecraft: Pi Edition
if(BUILD_ARM_COMPONENTS AND NOT MCPI_OPEN_SOURCE_ONLY)
    add_subdirectory(minecraft-pi)
endif()
# Zenity (Minimal Build)
if(BUILD_NATIVE_COMPONENTS AND NOT MCPI_SERVER_MODE)
    add_subdirectory(zenity)
endif()
# LIEF
if(BUILD_NATIVE_COMPONENTS OR (BUILD_ARM_COMPONENTS AND NOT MCPI_SERVER_MODE AND NOT MCPI_USE_MEDIA_LAYER_PROXY))
    add_subdirectory(LIEF)
endif()
# QEMU
if(BUILD_NATIVE_COMPONENTS AND NOT (CMAKE_SYSTEM_PROCESSOR MATCHES "arm*" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64"))
    add_subdirectory(qemu)
endif()
# GLFW
if(BUILD_MEDIA_LAYER_CORE AND NOT MCPI_HEADLESS_MODE)
    add_subdirectory(glfw)
endif()
# GLES Compatibility Layer
if(BUILD_MEDIA_LAYER_CORE AND NOT MCPI_HEADLESS_MODE AND MCPI_USE_GLES1_COMPATIBILITY_LAYER)
    add_subdirectory(gles-compatibility-layer)
endif()
# UTF8-CPP
add_subdirectory(utf8cpp)
# Symbol Prcoessor
if(BUILD_ARM_COMPONENTS)
    add_subdirectory(symbol-processor)
endif()
