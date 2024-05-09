project(mods)

# Common Sources
set(SRC
    # compat
    src/compat/compat.cpp
    src/compat/egl.cpp
    src/compat/x11.cpp
    src/compat/bcm_host.cpp
    # readdir
    src/readdir/readdir.cpp
    # feature
    src/feature/feature.cpp
    # version
    src/version/version.cpp
    # chat
    src/chat/chat.cpp
    src/chat/ui.cpp
    # creative
    src/creative/creative.cpp
    # game-mode
    src/game-mode/game-mode.cpp
    src/game-mode/ui.cpp
    # override
    src/override/override.cpp
    # death
    src/death/death.cpp
    # misc
    src/misc/misc.cpp
    src/misc/logging.cpp
    src/misc/api.cpp
    # options
    src/options/options.cpp
    src/options/ui.cpp
    # bucket
    src/bucket/bucket.cpp
    # cake
    src/cake/cake.cpp
    # home
    src/home/home.cpp
    # test
    src/test/test.cpp
    # init
    src/init/init.cpp
)

# Server-Only Sources
if(MCPI_SERVER_MODE)
    list(APPEND SRC
        # server
        src/server/server.cpp
        src/server/server_properties.cpp
    )
else()
    list(APPEND SRC
        # multiplayer
        src/multiplayer/multiplayer.cpp
        # benchmark
        src/benchmark/benchmark.cpp
    )
endif()

# Headless-Only Sources
if(MCPI_HEADLESS_MODE)
    list(APPEND SRC
        # textures
        src/textures/headless.cpp
    )
else()
    list(APPEND SRC
        # sound
        src/sound/sound.cpp
        src/sound/repository.cpp
        # camera
        src/camera/camera.cpp
        # input
        src/input/input.cpp
        src/input/bow.cpp
        src/input/attack.cpp
        src/input/toggle.cpp
        src/input/misc.cpp
        src/input/drop.cpp
        src/input/crafting.cpp
        # sign
        src/sign/sign.cpp
        # touch
        src/touch/touch.cpp
        # atlas
        src/atlas/atlas.cpp
        # title-screen
        src/title-screen/title-screen.cpp
        src/title-screen/splashes.txt # Show In IDE
        # skin
        src/skin/skin.cpp
        src/skin/loader.cpp
        # screenshot
        src/screenshot/screenshot.cpp
        # textures
        src/textures/textures.cpp
        src/textures/lava.cpp
        # text-input-box
        src/text-input-box/TextInputBox.cpp
        src/text-input-box/TextInputScreen.cpp
        # fps
        src/fps/fps.cpp
    )
    # Install Splashes
    install(
        FILES "src/title-screen/splashes.txt"
        DESTINATION "${MCPI_INSTALL_DIR}/data"
    )
endif()

# Build
add_library(mods SHARED ${SRC})

# Install
install(TARGETS mods DESTINATION "${MCPI_INSTALL_DIR}/mods")
# SDK
install(TARGETS mods EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")

# Dependencies
target_link_libraries(mods symbols reborn-patch media-layer-core dl pthread)
if(NOT MCPI_HEADLESS_MODE)
    target_link_libraries(mods stb_image)
endif()

# Headers
target_include_directories(
    mods
    PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${MCPI_SDK_INCLUDE_DIR}/mods>"
)
# SDK
install(DIRECTORY "include/" DESTINATION "${MCPI_SDK_INCLUDE_DIR}/mods")
