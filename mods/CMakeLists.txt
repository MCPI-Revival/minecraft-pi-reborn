project(mods)

# Build
add_library(mods SHARED
    # compat
    src/compat/compat.cpp
    src/compat/readdir.cpp
    src/compat/sdl.cpp
    src/compat/stubs/egl.cpp
    src/compat/stubs/x11.cpp
    src/compat/stubs/bcm_host.cpp
    src/compat/stubs/sdl.cpp
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
    src/misc/home.cpp
    src/misc/base64.cpp
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
    src/textures/tinting.cpp
    # fps
    src/fps/fps.cpp
    # server
    src/server/server.cpp
    src/server/commands.cpp
    src/server/properties.cpp
    src/server/blacklist.cpp
    src/server/playerdata.cpp
    # multiplayer
    src/multiplayer/server-list.cpp
    src/multiplayer/raknet.cpp
    src/multiplayer/misc.cpp
    src/multiplayer/syncing.cpp
    src/multiplayer/inventory.cpp
    src/multiplayer/loading/init.cpp
    src/multiplayer/loading/packets.cpp
    src/multiplayer/loading/thread.cpp
    src/multiplayer/loading/misc.cpp
    src/multiplayer/loading/terrain.cpp
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
    # api
    src/api/api.cpp
    src/api/events.cpp
    src/api/compat.cpp
    src/api/misc.cpp
    src/api/socket.cpp
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
