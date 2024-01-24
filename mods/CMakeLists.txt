project(mods)

# Common Sources
set(SRC
    # compat
    src/compat/compat.c
    src/compat/egl.c
    src/compat/x11.c
    src/compat/bcm_host.c
    # readdir
    src/readdir/readdir.c
    # feature
    src/feature/feature.c
    # version
    src/version/version.cpp
    # chat
    src/chat/chat.cpp
    src/chat/ui.c
    # creative
    src/creative/creative.cpp
    # game-mode
    src/game-mode/game-mode.c
    src/game-mode/ui.cpp
    # override
    src/override/override.c
    # death
    src/death/death.cpp
    # misc
    src/misc/misc.c
    src/misc/misc.cpp
    src/misc/logging.cpp
    src/misc/api.cpp
    # options
    src/options/options.c
    src/options/options.cpp
    # bucket
    src/bucket/bucket.cpp
    # home
    src/home/home.c
    # test
    src/test/test.c
    # init
    src/init/init.c
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
        src/input/bow.c
        src/input/attack.c
        src/input/toggle.c
        src/input/misc.c
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
        # skin
        src/skin/skin.cpp
        src/skin/loader.cpp
        # screenshot
        src/screenshot/screenshot.c
        # textures
        src/textures/textures.cpp
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
