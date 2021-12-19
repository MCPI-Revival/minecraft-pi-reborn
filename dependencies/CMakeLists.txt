project(dependencies)

# ZLib
add_subdirectory(zlib)
# LibPNG
add_subdirectory(libpng)
# Minecraft: Pi Edition
if(NOT MCPI_OPEN_SOURCE_ONLY)
    add_subdirectory(minecraft-pi)
endif()
