#pragma once

#include <string>
#include <vector>
#include <optional>

#include <symbols/minecraft.h>

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
