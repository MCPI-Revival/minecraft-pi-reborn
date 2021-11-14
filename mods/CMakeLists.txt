project(mods)

## Setup

# Disable C++11 String ABI
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

## Mods

add_library(compat SHARED src/compat/compat.c src/compat/egl.c src/compat/x11.c)
target_link_libraries(compat reborn media-layer-core)

add_library(readdir SHARED src/readdir/readdir.c)

add_library(feature SHARED src/feature/feature.c)
target_link_libraries(feature reborn)

add_library(version SHARED src/version/version.cpp)
target_link_libraries(version reborn symbols)

add_library(chat SHARED src/chat/chat.cpp src/chat/ui.c)
target_link_libraries(chat reborn symbols feature pthread)

if(MCPI_SERVER_MODE)
    add_library(server SHARED src/server/server.cpp src/server/server_properties.cpp)
    target_link_libraries(server reborn symbols feature home misc compat dl media-layer-core pthread)
else()
    target_link_libraries(compat input sign chat home dl)

    target_link_libraries(chat input)

    add_library(multiplayer SHARED src/multiplayer/multiplayer.cpp)
    target_link_libraries(multiplayer reborn symbols home feature)

    add_library(sound SHARED src/sound/sound.cpp src/sound/repository.cpp)
    target_link_libraries(sound reborn symbols feature override media-layer-core)

    add_library(camera SHARED src/camera/camera.cpp)
    target_link_libraries(camera reborn symbols media-layer-core feature home)

    add_library(input SHARED src/input/input.cpp src/input/bow.c src/input/attack.c src/input/toggle.c src/input/misc.c src/input/drop.cpp)
    target_link_libraries(input reborn symbols creative feature media-layer-core)

    add_library(sign SHARED src/sign/sign.cpp)
    target_link_libraries(sign reborn symbols feature input)

    add_library(creative SHARED src/creative/creative.cpp)
    target_link_libraries(creative reborn symbols feature)

    add_library(touch SHARED src/touch/touch.c)
    target_link_libraries(touch reborn symbols feature)

    add_library(override SHARED src/override/override.c)
    target_link_libraries(override reborn symbols dl home)

    add_library(textures SHARED src/textures/textures.cpp)
    target_link_libraries(textures reborn symbols media-layer-core feature)

    add_library(atlas SHARED src/atlas/atlas.cpp)
    target_link_libraries(atlas reborn symbols feature GLESv1_CM)

    if(NOT MCPI_HEADLESS_MODE)
        add_library(benchmark SHARED src/benchmark/benchmark.cpp)
        target_link_libraries(benchmark reborn symbols compat misc media-layer-core)
    endif()
endif()

add_library(game-mode SHARED src/game-mode/game-mode.c src/game-mode/game-mode.cpp)
target_link_libraries(game-mode reborn symbols feature)

add_library(death SHARED src/death/death.cpp)
target_link_libraries(death reborn symbols feature)

add_library(misc SHARED src/misc/misc.c src/misc/misc.cpp src/misc/logging.cpp)
target_link_libraries(misc reborn symbols feature GLESv1_CM)

add_library(options SHARED src/options/options.c)
target_link_libraries(options reborn symbols feature)

add_library(home SHARED src/home/home.c)
target_link_libraries(home reborn symbols)

add_library(test SHARED src/test/test.c)
target_link_libraries(test reborn home)

add_library(init SHARED src/init/init.c)
target_link_libraries(init compat game-mode misc death options chat home version test media-layer-core)
if(MCPI_SERVER_MODE)
    target_link_libraries(init server)
else()
    target_link_libraries(init multiplayer sound camera input sign creative touch textures atlas)
    if(NOT MCPI_HEADLESS_MODE)
        target_link_libraries(init benchmark)
    endif()
endif()

## Install Mods
install(TARGETS init compat readdir feature game-mode misc death options chat home version test DESTINATION "${MCPI_INSTALL_DIR}/mods")
if(MCPI_SERVER_MODE)
    install(TARGETS server DESTINATION "${MCPI_INSTALL_DIR}/mods")
else()
    install(TARGETS multiplayer sound override camera input sign creative touch textures atlas DESTINATION "${MCPI_INSTALL_DIR}/mods")
    if(NOT MCPI_HEADLESS_MODE)
        install(TARGETS benchmark DESTINATION "${MCPI_INSTALL_DIR}/mods")
    endif()
endif()
