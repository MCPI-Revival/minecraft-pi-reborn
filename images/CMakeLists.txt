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

# Chest Model
install(
    FILES "chest.png"
    DESTINATION "${MCPI_INSTALL_DIR}/data/images/item"
)

# Icon
set(ICON_DIR "${MCPI_SHARE_DIR}/icons/hicolor/1024x1024/apps")
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
