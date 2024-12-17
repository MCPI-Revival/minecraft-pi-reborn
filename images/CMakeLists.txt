project(images)

# Logo
install(
    FILES "modern_logo.png"
    DESTINATION "${MCPI_INSTALL_DIR}/data/images/gui"
)

# Title Background
install(
    FILES "background.png"
    DESTINATION "${MCPI_INSTALL_DIR}/data/images/gui"
    RENAME "titleBG.png"
)

# Mojang Textures
install(
    FILES "mojang/chest.png"
    DESTINATION "${MCPI_INSTALL_DIR}/data/images/item"
)
install(
    FILES "mojang/shadow.png" "mojang/vignette.png" "mojang/grasscolor.png"
    DESTINATION "${MCPI_INSTALL_DIR}/data/images/misc"
)

# Icon
set(ICON_DIR "${MCPI_SHARE_DIR}/icons/hicolor/512x512/apps")
install(
    FILES "icon.png"
    DESTINATION "${ICON_DIR}"
    RENAME "${MCPI_APP_ID}.png"
)

# AppImage
if(MCPI_IS_APPIMAGE_BUILD)
    install_symlink("${ICON_DIR}/${MCPI_APP_ID}.png" "${MCPI_APP_ID}.png")
    install_symlink("${MCPI_APP_ID}.png" ".DirIcon")
endif()
