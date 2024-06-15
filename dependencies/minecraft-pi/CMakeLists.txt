project(minecraft-pi)

include(FetchContent)

## Minecraft: Pi Edition

# Download
FetchContent_Declare(
    minecraft-pi
    URL "${CMAKE_CURRENT_SOURCE_DIR}/minecraft-pi-0.1.1.tar.gz"
)
FetchContent_Populate(minecraft-pi)

# Install
install(
    DIRECTORY "${minecraft-pi_SOURCE_DIR}/"
    DESTINATION "${MCPI_INSTALL_DIR}/game"
    USE_SOURCE_PERMISSIONS
    REGEX "api" EXCLUDE
)
install_symlink("game/minecraft-pi" "${MCPI_INSTALL_DIR}/minecraft-pi")
