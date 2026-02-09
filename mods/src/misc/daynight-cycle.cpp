#include "internal.h"

#include <libreborn/patch.h>

#include <symbols/Dimension.h>
#include <symbols/DimensionFactory.h>
#include <symbols/StartGamePacket.h>
#include <symbols/Minecraft.h>

#include <mods/feature/feature.h>
#include <mods/misc/misc.h>
#include <mods/multiplayer/packets.h>

// Flags
static bool force_daynight_cycle;
static bool force_no_daynight_cycle;
static bool has_read_flags = false;
#define read_flags(func) \
    force_daynight_cycle = StartGameFlags::func(StartGameFlags::FORCE_DAYNIGHT_CYCLE); \
    force_no_daynight_cycle = StartGameFlags::func(StartGameFlags::FORCE_NO_DAYNIGHT_CYCLE); \
    has_read_flags = true

// Allow Overriding The Dimension
static Dimension *DimensionFactory_createDefaultDimension_injection(DimensionFactory_createDefaultDimension_t original, LevelData *data) {
    if (!has_read_flags) {
        IMPOSSIBLE();
    } else if (force_daynight_cycle) {
        return Dimension::getNew(10);
    } else if (force_no_daynight_cycle) {
        return Dimension::getNew(0);
    } else {
        // Call Original Method
        return original(data);
    }
}

// Receive A Dimension Override From The Server
static void StartGamePacket_handle_injection(StartGamePacket_handle_t original, StartGamePacket *self, const RakNet_RakNetGUID &guid, NetEventCallback *callback) {
    // Call Original Method
    read_flags(has_received_from_server);
    original(self, guid, callback);
    has_read_flags = false;
}

// Override The Dimension Locally
static void Minecraft_selectLevel_injection(Minecraft_selectLevel_t original, Minecraft *self, const std::string &level_dir, const std::string &level_name, const LevelSettings &settings) {
    // Call Original Method
    read_flags(will_send_to_clients);
    original(self, level_dir, level_name, settings);
    has_read_flags = false;
}

// Init
void _init_misc_daynight_cycle() {
    // Override The Level's Dimension
    overwrite_calls(DimensionFactory_createDefaultDimension, DimensionFactory_createDefaultDimension_injection);
    overwrite_calls(StartGamePacket_handle, StartGamePacket_handle_injection);
    overwrite_calls(Minecraft_selectLevel, Minecraft_selectLevel_injection);

    // Force Day/Ight Cycle
    if (feature_has("Force Day/Night Cycle", server_force_daynight_cycle)) {
        StartGameFlags::send_to_clients(StartGameFlags::FORCE_DAYNIGHT_CYCLE);
    }
}