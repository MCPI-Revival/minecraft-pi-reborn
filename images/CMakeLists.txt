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
set(ICON_NAME "${MCPI_APP_ID}.png")
install(
    FILES "icon.png"
    DESTINATION "${ICON_DIR}"
    RENAME "${ICON_NAME}"
)
set(ICON_PATH "${ICON_DIR}/${ICON_NAME}")
cmake_path(RELATIVE_PATH ICON_PATH BASE_DIRECTORY "${MCPI_INSTALL_DIR}")
target_compile_definitions(launcher-util PRIVATE "ICON_PATH=\"${ICON_PATH}\"")

# AppImage
if(MCPI_IS_APPIMAGE_BUILD)
    install_symlink("${ICON_DIR}/${MCPI_APP_ID}.png" "${MCPI_APP_ID}.png")
    install_symlink("${MCPI_APP_ID}.png" ".DirIcon")
endif()
