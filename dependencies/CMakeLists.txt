project(dependencies)

# LibPNG
add_subdirectory(libpng)
# Minecraft: Pi Edition
if(BUILD_ARM_COMPONENTS AND NOT MCPI_OPEN_SOURCE_ONLY)
    add_subdirectory(minecraft-pi)
endif()
# Zenity (Minimal Build)
if(BUILD_NATIVE_COMPONENTS AND NOT MCPI_SERVER_MODE)
    add_subdirectory(zenity)
endif()
