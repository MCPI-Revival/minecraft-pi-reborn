#include "internal.h"

#include <libreborn/patch.h>

#include <optional>

#include <symbols/Dimension.h>
#include <symbols/DimensionFactory.h>
#include <symbols/StartGamePacket.h>
#include <symbols/RakNet_BitStream.h>
#include <symbols/Minecraft.h>

#include <mods/feature/feature.h>
#include <mods/misc/misc.h>

// Receive A Dimension Override From The Server
static std::optional<int> received_dimension_override;
static Dimension *DimensionFactory_createDefaultDimension_injection(DimensionFactory_createDefaultDimension_t original, LevelData *data) {
    if (received_dimension_override.has_value()) {
        // Use Override
        const int id = received_dimension_override.value();
        received_dimension_override.reset();
        Dimension *ret = Dimension::getNew(id);
        if (!ret) {
            ERR("Invalid Dimension ID: %i", id);
        }
        DEBUG("Received Dimension Override: %i", id);
        return ret;
    } else {
        // Call Original Method
        return original(data);
    }
}
static void StartGamePacket_read_injection(StartGamePacket_read_t original, StartGamePacket *self, RakNet_BitStream *stream) {
    // Call Original Method
    original(self, stream);
    // Read Override
    int id;
    if (stream->Read_int(&id)) {
        received_dimension_override = id;
    } else {
        received_dimension_override.reset();
    }
}

// Override The Dimension
static std::optional<int> local_dimension_override;
static void Minecraft_selectLevel_injection(Minecraft_selectLevel_t original, Minecraft *self, const std::string &level_dir, const std::string &level_name, const LevelSettings &settings) {
    // Enable Override
    received_dimension_override = local_dimension_override;
    // Call Original Method
    original(self, level_dir, level_name, settings);
}
static void StartGamePacket_write_injection(StartGamePacket_write_t original, StartGamePacket *self, RakNet_BitStream *stream) {
    // Call Original Method
    original(self, stream);
    // Send Override
    if (local_dimension_override.has_value()) {
        int id = local_dimension_override.value();
        stream->Write_int(&id);
    }
}

// Init
void _init_misc_daynight_cycle() {
    // Receive A Dimension Override
    overwrite_calls(DimensionFactory_createDefaultDimension, DimensionFactory_createDefaultDimension_injection);
    overwrite_calls(StartGamePacket_read, StartGamePacket_read_injection);

    // Send A Dimension Override
    overwrite_calls(Minecraft_selectLevel, Minecraft_selectLevel_injection);
    overwrite_calls(StartGamePacket_write, StartGamePacket_write_injection);

    // Force Day/Ight Cycle
    if (feature_has("Force Day/Night Cycle", server_force_daynight_cycle)) {
        misc_run_on_init([](Minecraft *) {
            local_dimension_override = 10;
        });
    }
}