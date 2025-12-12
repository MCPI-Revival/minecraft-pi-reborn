#pragma once

#include <string>
#include <vector>
#include <optional>

#include <symbols/CommandServer.h>
#include <symbols/Vec3.h>
#include <symbols/Minecraft.h>
#include <symbols/Level.h>
#include <symbols/Entity.h>
#include <symbols/TileEntity.h>
#include <symbols/SignTileEntity.h>
#include <symbols/Mob.h>
#include <symbols/ItemInstance.h>
#include <symbols/ItemEntity.h>
#include <symbols/LocalPlayer.h>
#include <symbols/Item.h>
#include <symbols/RakNetInstance.h>
#include <symbols/Packet.h>
#include <symbols/SetEntityMotionPacket.h>
#include <symbols/TileEvent.h>
#include <symbols/Arrow.h>
#include <symbols/Throwable.h>
#include <symbols/GameMode.h>
#include <symbols/CreatorMode.h>
#include <symbols/ItemInHandRenderer.h>
#include <symbols/LevelListener.h>
#include <symbols/LevelRenderer.h>
#include <symbols/MovePlayerPacket.h>
#include <symbols/ClientSideNetworkHandler.h>
#include <symbols/ServerSideNetworkHandler.h>
#include <symbols/ChatPacket.h>

MCPI_INTERNAL extern bool api_compat_mode;

static constexpr int no_entity_id = -1;
static constexpr char arg_separator = ','; // Used For Arguments And Tuples
static constexpr char list_separator = '|'; // Used For Lists

MCPI_INTERNAL std::string api_get_input(std::string message);
MCPI_INTERNAL std::string api_get_output(std::string message, bool replace_comma);
MCPI_INTERNAL std::string api_join_outputs(const std::vector<std::string> &pieces, char separator);

MCPI_INTERNAL void api_convert_to_outside_entity_type(int &type);
MCPI_INTERNAL void api_convert_to_mcpi_entity_type(int &type);

MCPI_INTERNAL void _init_api_events();
MCPI_INTERNAL void _init_api_misc();
MCPI_INTERNAL void _init_api_socket();

MCPI_INTERNAL std::string api_handle_event_command(CommandServer *server, const ConnectedClient &client, const std::string_view &cmd, std::optional<int> id);
MCPI_INTERNAL void api_free_event_data(int sock);
MCPI_INTERNAL void api_free_all_event_data();

MCPI_INTERNAL extern bool api_suppress_chat_events;
