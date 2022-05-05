project(dependencies)

# ZLib
if(BUILD_ARM_COMPONENTS)
    add_subdirectory(zlib)
endif()
# LibPNG
if(BUILD_ARM_COMPONENTS)
    add_subdirectory(libpng)
endif()
# Minecraft: Pi Edition
if(BUILD_ARM_COMPONENTS AND NOT MCPI_OPEN_SOURCE_ONLY)
    add_subdirectory(minecraft-pi)
endif()
# GLFW
if(BUILD_NATIVE_COMPONENTS AND NOT MCPI_HEADLESS_MODE)
    add_subdirectory(glfw)
endif()
# Zenity (Minimal Build)
if(BUILD_NATIVE_COMPONENTS AND NOT MCPI_HEADLESS_MODE)
    add_subdirectory(zenity)
endif()
# Sysroot
if(BUILD_ARM_COMPONENTS AND MCPI_IS_MIXED_BUILD)
    add_subdirectory(armhf-sysroot)
endif()
