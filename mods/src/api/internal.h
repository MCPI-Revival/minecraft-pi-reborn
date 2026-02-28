#pragma once

#include <string>
#include <vector>

extern bool api_compat_mode;

static constexpr int no_entity_id = -1;
static constexpr char arg_separator = ','; // Used For Arguments And Tuples
static constexpr char list_separator = '|'; // Used For Lists

std::string api_get_input(std::string message);
std::string api_get_output(std::string message, bool replace_comma);
std::string api_join_outputs(const std::vector<std::string> &pieces, char separator);

void api_convert_to_outside_entity_type(int &type);
void api_convert_to_mcpi_entity_type(int &type);

void _init_api_commands();
void _init_api_misc();
void _init_api_socket();

void api_free_event_data(int sock);
void api_free_all_event_data();

extern bool api_suppress_chat_events;
