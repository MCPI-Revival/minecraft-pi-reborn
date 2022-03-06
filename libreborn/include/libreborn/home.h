#pragma once

// Minecraft Pi Game Data Root
#ifndef MCPI_SERVER_MODE
// Store Game Data In "~/.minecraft-pi" Instead Of "~/.minecraft" To Avoid Conflicts
#define HOME_SUBDIRECTORY_FOR_GAME_DATA "/.minecraft-pi"
#else
// Store Game Data In $HOME Root (In Server Mode, $HOME Is Changed To The Launch Directory)
#define HOME_SUBDIRECTORY_FOR_GAME_DATA ""
#endif
