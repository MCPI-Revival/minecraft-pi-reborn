project(mods)

# Build
add_library(mods SHARED
    # compat
    src/compat/compat.cpp
    src/compat/egl.cpp
    src/compat/x11.cpp
    src/compat/bcm_host.cpp
    src/compat/sdl.cpp
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
    src/misc/graphics.cpp
    src/misc/ui.cpp
    src/misc/tinting.cpp
    src/misc/home.cpp
    # extend
    src/extend/Screen.cpp
    src/extend/DynamicTexture.cpp
    # options
    src/options/options.cpp
    src/options/ui.cpp
    src/options/info.cpp
    # bucket
    src/bucket/bucket.cpp
    # cake
    src/cake/cake.cpp
    # touch
    src/touch/touch.cpp
    # text-input-box
    src/text-input-box/TextInputBox.cpp
    src/text-input-box/TextInputScreen.cpp
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
    src/input/keys.cpp
    # sign
    src/sign/sign.cpp
    # atlas
    src/atlas/atlas.cpp
    # title-screen
    src/title-screen/title-screen.cpp
    src/title-screen/splashes.cpp
    src/title-screen/splashes.txt # Show In IDE
    src/title-screen/welcome.cpp
    # skin
    src/skin/skin.cpp
    src/skin/loader.cpp
    # screenshot
    src/screenshot/screenshot.cpp
    # textures
    src/textures/textures.cpp
    src/textures/lava.cpp
    src/textures/headless.cpp
    # fps
    src/fps/fps.cpp
    # server
    src/server/server.cpp
    src/server/server_properties.cpp
    # multiplayer
    src/multiplayer/multiplayer.cpp
    # benchmark
    src/benchmark/benchmark.cpp
    # init
    src/init/init.cpp
    # f3
    src/f3/f3.cpp
    # multidraw
    src/multidraw/glue.cpp
    src/multidraw/buffer.cpp
    src/multidraw/storage.cpp
    # classic-ui
    src/classic-ui/classic-ui.cpp
    # shading
    src/shading/init.cpp
    src/shading/tesselator.cpp
    src/shading/lighting.cpp
    src/shading/normals.cpp
)

# Install Splashes
install(
    FILES "src/title-screen/splashes.txt"
    DESTINATION "${MCPI_INSTALL_DIR}/data"
)

# Headers
setup_header_dirs(mods "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Install
set(old_lib_dir "${MCPI_LIB_DIR}")
set(MCPI_LIB_DIR "${MCPI_INSTALL_DIR}/mods")
setup_library(mods TRUE TRUE)
set(MCPI_LIB_DIR "${old_lib_dir}")

# Dependencies
target_link_libraries(mods
    PUBLIC
        symbols
        reborn-patch
        media-layer-core
    PRIVATE
        stb_image
        dl
        pthread
)
