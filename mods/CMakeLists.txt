project(mods)

## Setup

# Disable C++11 String ABI
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

## Mods

add_library(compat SHARED src/compat/compat.c src/compat/egl.c src/compat/x11.c)
target_link_libraries(compat feature input sign media-layer-core dl)

add_library(readdir SHARED src/readdir/readdir.c)

add_library(feature SHARED src/feature/feature.c)
target_link_libraries(feature reborn)

add_library(version SHARED src/version/version.cpp)
target_link_libraries(version reborn)

if(MCPI_SERVER_MODE)
    add_library(server SHARED src/server/server.cpp src/server/server_properties.cpp)
    target_link_libraries(server reborn feature home compat version dl media-layer-core pthread)
else()
    add_library(multiplayer SHARED src/multiplayer/multiplayer.cpp)
    target_link_libraries(multiplayer reborn home)
endif()

add_library(camera SHARED src/camera/camera.cpp)
target_link_libraries(camera reborn media-layer-core)

add_library(game-mode SHARED src/game-mode/game-mode.c src/game-mode/game-mode.cpp)
target_link_libraries(game-mode reborn)

add_library(sign SHARED src/sign/sign.cpp)
target_link_libraries(sign reborn feature)

add_library(input SHARED src/input/input.c)
target_link_libraries(input reborn feature media-layer-core chat sign)

add_library(death SHARED src/death/death.cpp)
target_link_libraries(death reborn)

add_library(misc SHARED src/misc/misc.c src/misc/misc.cpp)
target_link_libraries(misc reborn feature)

add_library(options SHARED src/options/options.c)
target_link_libraries(options reborn feature)

add_library(touch SHARED src/touch/touch.c)
target_link_libraries(touch reborn feature)

add_library(override SHARED src/override/override.c)
target_link_libraries(override reborn dl home)

add_library(textures SHARED src/textures/textures.cpp)
target_link_libraries(textures reborn feature GLESv1_CM)

add_library(chat SHARED src/chat/chat.cpp src/chat/ui.c)
target_link_libraries(chat reborn pthread)

add_library(home SHARED src/home/home.c)
target_link_libraries(home reborn)

add_library(test SHARED src/test/test.c)
target_link_libraries(test reborn home)

add_library(init SHARED src/init/init.c)
target_link_libraries(init compat game-mode camera input sign misc death options touch textures chat home version test)
if(MCPI_SERVER_MODE)
    target_link_libraries(init server)
else()
    target_link_libraries(init multiplayer)
endif()

## Install Mods
install(TARGETS init compat readdir feature override game-mode camera input sign misc death options touch textures chat home version test DESTINATION "${MCPI_INSTALL_DIR}/mods")
if(MCPI_SERVER_MODE)
    install(TARGETS server DESTINATION "${MCPI_INSTALL_DIR}/mods")
else()
    install(TARGETS multiplayer DESTINATION "${MCPI_INSTALL_DIR}/mods")
endif()
