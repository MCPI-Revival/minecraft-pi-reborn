project(mods)

## Setup

# Disable C++11 String ABI
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

## Mods

add_library(compat SHARED src/compat/compat.c src/compat/egl.c)
target_link_libraries(compat feature input media-layer-core dl)

add_library(readdir SHARED src/readdir/readdir.c)

add_library(feature SHARED src/feature/feature.c)
target_link_libraries(feature reborn)

if(MCPI_SERVER_MODE)
    add_library(server SHARED src/server/server.cpp src/server/server_properties.cpp)
    target_link_libraries(server reborn feature home compat dl media-layer-core pthread)
endif()

add_library(camera SHARED src/camera/camera.cpp)
target_link_libraries(camera reborn media-layer-core)

add_library(game_mode SHARED src/game_mode/game_mode.c src/game_mode/game_mode.cpp)
target_link_libraries(game_mode reborn)

add_library(input SHARED src/input/input.c src/input/input.cpp)
target_link_libraries(input reborn feature media-layer-core chat)

add_library(misc SHARED src/misc/misc.c src/misc/misc.cpp)
target_link_libraries(misc reborn feature util)

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
target_link_libraries(init compat game_mode camera input misc options touch textures chat home test)
if(MCPI_SERVER_MODE)
    target_link_libraries(init server)
endif()

## Install Mods
install(TARGETS init compat readdir feature override game_mode camera input misc options touch textures chat home test DESTINATION "${MCPI_INSTALL_DIR}/mods")
if(MCPI_SERVER_MODE)
    install(TARGETS server DESTINATION "${MCPI_INSTALL_DIR}/mods")
endif()
