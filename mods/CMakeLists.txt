project(mods)

## Headers
add_library(mods-headers INTERFACE)
target_include_directories(
    mods-headers
    INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${MCPI_SDK_INCLUDE_DIR}/mods>"
)
# SDK
install(TARGETS mods-headers EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")
install(DIRECTORY "include/" DESTINATION "${MCPI_SDK_INCLUDE_DIR}/mods")

## Mods

add_library(compat SHARED src/compat/compat.c src/compat/egl.c src/compat/x11.c src/compat/bcm_host.c)
target_link_libraries(compat mods-headers reborn-patch media-layer-core)

add_library(readdir SHARED src/readdir/readdir.c)

add_library(feature SHARED src/feature/feature.c)
target_link_libraries(feature mods-headers reborn-patch)

add_library(version SHARED src/version/version.cpp)
target_link_libraries(version mods-headers reborn-patch symbols)

add_library(chat SHARED src/chat/chat.cpp src/chat/ui.c)
target_link_libraries(chat mods-headers reborn-patch symbols feature)

add_library(creative SHARED src/creative/creative.cpp)
target_link_libraries(creative mods-headers reborn-patch symbols feature)

add_library(game-mode SHARED src/game-mode/game-mode.c src/game-mode/ui.cpp)
target_link_libraries(game-mode mods-headers reborn-patch symbols feature)

if(MCPI_SERVER_MODE)
    add_library(server SHARED src/server/server.cpp src/server/server_properties.cpp)
    target_link_libraries(server mods-headers reborn-patch symbols feature home misc compat dl media-layer-core pthread)
else()
    target_link_libraries(compat input sign chat home dl)

    target_link_libraries(chat input media-layer-core pthread)

    target_link_libraries(game-mode pthread media-layer-core)

    add_library(multiplayer SHARED src/multiplayer/multiplayer.cpp)
    target_link_libraries(multiplayer mods-headers reborn-patch symbols home feature)

    add_library(sound SHARED src/sound/sound.cpp src/sound/repository.cpp)
    target_link_libraries(sound mods-headers reborn-patch symbols feature override media-layer-core)

    add_library(camera SHARED src/camera/camera.cpp)
    target_link_libraries(camera mods-headers reborn-patch symbols media-layer-core feature home)

    add_library(input SHARED src/input/input.cpp src/input/bow.c src/input/attack.c src/input/toggle.c src/input/misc.c src/input/drop.cpp)
    target_link_libraries(input mods-headers reborn-patch symbols creative feature media-layer-core)

    add_library(sign SHARED src/sign/sign.cpp)
    target_link_libraries(sign mods-headers reborn-patch symbols feature input)

    add_library(touch SHARED src/touch/touch.cpp)
    target_link_libraries(touch mods-headers reborn-patch symbols feature)

    add_library(textures SHARED src/textures/textures.cpp)
    target_link_libraries(textures mods-headers reborn-patch symbols media-layer-core feature misc)

    add_library(atlas SHARED src/atlas/atlas.cpp)
    target_link_libraries(atlas mods-headers reborn-patch symbols feature GLESv1_CM)

    add_library(benchmark SHARED src/benchmark/benchmark.cpp)
    target_link_libraries(benchmark mods-headers reborn-patch symbols compat misc media-layer-core)
endif()

add_library(override SHARED src/override/override.c)
target_link_libraries(override mods-headers reborn-patch symbols dl home)

add_library(death SHARED src/death/death.cpp)
target_link_libraries(death mods-headers reborn-patch symbols feature)

add_library(misc SHARED src/misc/misc.c src/misc/misc.cpp src/misc/logging.cpp src/misc/api.cpp)
target_link_libraries(misc mods-headers reborn-patch symbols media-layer-core feature GLESv1_CM)

add_library(options SHARED src/options/options.c src/options/options.cpp)
target_link_libraries(options mods-headers reborn-patch symbols feature home)

add_library(home SHARED src/home/home.c)
target_link_libraries(home mods-headers reborn-patch symbols)

add_library(test SHARED src/test/test.c)
target_link_libraries(test mods-headers reborn-patch home)

add_library(init SHARED src/init/init.c)
target_link_libraries(init mods-headers compat game-mode misc death options chat creative home version test media-layer-core)
if(MCPI_SERVER_MODE)
    target_link_libraries(init server)
else()
    target_link_libraries(init multiplayer sound camera input sign touch textures atlas benchmark)
endif()

## Install Mods
set(MODS_TO_INSTALL init compat readdir feature game-mode misc override death options chat creative home version test)
if(MCPI_SERVER_MODE)
    list(APPEND MODS_TO_INSTALL server)
else()
    list(APPEND MODS_TO_INSTALL multiplayer sound camera input sign touch textures atlas benchmark)
endif()
install(TARGETS ${MODS_TO_INSTALL} DESTINATION "${MCPI_INSTALL_DIR}/mods")
# SDK
install(TARGETS ${MODS_TO_INSTALL} EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")
