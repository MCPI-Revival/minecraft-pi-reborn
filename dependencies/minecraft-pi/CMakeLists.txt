project(minecraft-pi)

include(FetchContent)

## Minecraft: Pi Edition

# Download
FetchContent_Declare(
    minecraft-pi
    URL "https://www.minecraft.net/content/dam/minecraft/edition-pi/minecraft-pi-0.1.1.tar.gz"
    URL_HASH "SHA256=e0d68918874cdd403de1fd399380ae2930913fcefdbf60a3fbfebb62e2cfacab"
)
FetchContent_Populate(minecraft-pi)

# Install
install(DIRECTORY "${minecraft-pi_SOURCE_DIR}/" DESTINATION "${MCPI_INSTALL_DIR}" USE_SOURCE_PERMISSIONS)

