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
    REGEX "data" EXCLUDE
)
if(NOT MCPI_HEADLESS_MODE)
    install(
        DIRECTORY "${minecraft-pi_SOURCE_DIR}/data/"
        DESTINATION "${MCPI_INSTALL_DIR}/game/data"
        USE_SOURCE_PERMISSIONS
    )
endif()
install_symlink("game/minecraft-pi" "${MCPI_INSTALL_DIR}/minecraft-pi")
