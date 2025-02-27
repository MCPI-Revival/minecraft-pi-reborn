#pragma once

#include <string>
#include <vector>
#include <optional>

#include <symbols/minecraft.h>

__attribute__((visibility("internal"))) extern bool api_compat_mode;

static constexpr int no_entity_id = -1;
static constexpr char arg_separator = ','; // Used For Arguments And Tuples
static constexpr char list_separator = '|'; // Used For Lists

__attribute__((visibility("internal"))) std::string api_get_input(std::string message);
__attribute__((visibility("internal"))) std::string api_get_output(std::string message, bool replace_comma);
__attribute__((visibility("internal"))) std::string api_join_outputs(const std::vector<std::string> &pieces, char separator);

__attribute__((visibility("internal"))) void api_convert_to_outside_entity_type(int &type);
__attribute__((visibility("internal"))) void api_convert_to_mcpi_entity_type(int &type);

__attribute__((visibility("internal"))) void _init_api_events();

__attribute__((visibility("internal"))) void api_clear_events(const ConnectedClient &client);
__attribute__((visibility("internal"))) void api_clear_events(const ConnectedClient &client, int id);

__attribute__((visibility("internal"))) std::string api_get_projectile_events(CommandServer *server, const ConnectedClient &client, std::optional<int> id);
__attribute__((visibility("internal"))) std::string api_get_chat_events(CommandServer *server, const ConnectedClient &client, std::optional<int> id);
__attribute__((visibility("internal"))) std::string api_get_block_hit_events(CommandServer *server, const ConnectedClient &client, std::optional<int> id);

__attribute__((visibility("internal"))) extern bool api_suppress_chat_events;
