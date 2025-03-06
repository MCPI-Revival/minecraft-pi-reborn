#include <ranges>

#include <libreborn/patch.h>

#include <mods/misc/misc.h>
#include <mods/chat/chat.h>
#include <mods/api/api.h>

#include "internal.h"

// Projectile Event
struct ProjectileHitEvent {
    int x;
    int y;
    int z;
    int owner_id;
    int target_id;
};
static std::string event_to_string(CommandServer *server, const ProjectileHitEvent &e) {
    // Offset Position
    int nx = e.x;
    int ny = e.y;
    int nz = e.z;
    server->pos_translator.to_int(nx, ny, nz);
    // Get Outputs
    std::vector pieces = {
        // Position
        std::to_string(nx),
        std::to_string(ny),
        std::to_string(nz)
    };
    // Needed For Compatibility
    if (api_compat_mode) {
        pieces.push_back("1");
    }
    // Owner
    Level *level = server->minecraft->level;
    std::string owner;
    if (api_compat_mode) {
        owner = api_get_output(misc_get_entity_name(level->getEntity(e.owner_id)), true);
    } else {
        owner = std::to_string(e.owner_id);
    }
    pieces.push_back(owner);
    // Target
    std::string target;
    if (api_compat_mode) {
        if (e.target_id != no_entity_id) {
            target = api_get_output(misc_get_entity_name(level->getEntity(e.target_id)), true);
        }
    } else {
        target = std::to_string(e.target_id);
    }
    pieces.push_back(target);
    // Return
    return api_join_outputs(pieces, arg_separator);
}

// Chat Message Event
struct ChatEvent {
    std::string message;
    int owner_id;
};
static std::string event_to_string(__attribute__((unused)) CommandServer *server, const ChatEvent &e) {
    return api_join_outputs({
        std::to_string(e.owner_id),
        api_get_output(e.message, true),
    }, arg_separator);
}

// Block Hit Event
static std::string event_to_string(CommandServer *server, const TileEvent &e) {
    // Offset Coordinates
    int x = e.x;
    int y = e.y;
    int z = e.z;
    server->pos_translator.to_int(x, y, z);
    // Output
    return api_join_outputs({
        // Position
        std::to_string(x),
        std::to_string(y),
        std::to_string(z),
        // Face
        std::to_string(e.face),
        // Entity ID
        std::to_string(e.owner_id)
    }, arg_separator);
}

// Track Event Queues Per Client
struct ExtraClientData {
    std::vector<ProjectileHitEvent> projectile_events;
    std::vector<ChatEvent> chat_events;
    std::vector<TileEvent> block_hit_events;
};
static std::unordered_map<int, ExtraClientData> extra_client_data;
static bool CommandServer__updateAccept_setSocketBlocking_injection(const int fd, const bool param_1) {
    // Client Was Created
    extra_client_data[fd] = ExtraClientData();
    return CommandServer::setSocketBlocking(fd, param_1);
}
void api_free_event_data(const int sock) {
    extra_client_data.erase(sock);
}
void api_free_all_event_data() {
    extra_client_data.clear();
}

// Clear All Events
static void clear_all_events(const ConnectedClient &client) {
    ExtraClientData &data = extra_client_data.at(client.sock);
    if (!api_compat_mode) {
        // Match RJ Bug
        data.projectile_events.clear();
    }
    data.chat_events.clear();
    data.block_hit_events.clear();
}

// Clear Events Produced By Given Entity
template <typename T>
static void _clear_events(std::vector<T> &data, const int id) {
    std::erase_if(data, [&id](const T &e) {
        return id == e.owner_id;
    });
}
static void clear_events(const ConnectedClient &client, const int id) {
    ExtraClientData &data = extra_client_data.at(client.sock);
    _clear_events(data.block_hit_events, id);
    _clear_events(data.chat_events, id);
    _clear_events(data.projectile_events, id);
}

// Get Events In Queue
template <typename T>
static std::string get_events(CommandServer *server, std::vector<T> &queue, const std::optional<int> id) {
    std::vector<std::string> out;
    typename std::vector<T>::iterator it = queue.begin();
    while (it != queue.end()) {
        const T &e = *it;
        if (id.has_value() && id.value() != e.owner_id) {
            // Skip Event
            ++it;
            continue;
        }
        // Output
        out.push_back(event_to_string(server, e));
        // Erase
        it = queue.erase(it);
    }
    return api_join_outputs(out, list_separator);
}
#define create_get_events(name) \
    static std::string get_##name##_events(CommandServer *server, const ConnectedClient &client, const std::optional<int> id) { \
        return get_events(server, extra_client_data.at(client.sock).name##_events, id); \
    }
create_get_events(projectile)
create_get_events(chat)
create_get_events(block_hit)
#undef create_get_events

// Push Event To Queue
template <typename T>
static void push_event(std::vector<T> ExtraClientData::*ptr, const T e) {
    for (ExtraClientData &data : extra_client_data | std::views::values) {
        (data.*ptr).push_back(e);
    }
}

// Arrow Hits
static void on_projectile_hit(Entity *shooter, const int x, const int y, const int z, const Entity *target) {
    if (shooter && shooter->isPlayer()) {
        push_event(&ExtraClientData::projectile_events, {
            .x = x,
            .y = y,
            .z = z,
            .owner_id = shooter->id,
            .target_id = target ? target->id : no_entity_id
        });
    }
}
static void on_projectile_hit(Entity *shooter, const Entity *target) {
    on_projectile_hit(shooter, int(target->x), int(target->y), int(target->z), target);
}
static Entity *current_shooter = nullptr;
static void Arrow_tick_injection(Arrow_tick_t original, Arrow *self) {
    current_shooter = self->level->getEntity(self->getAuxData());
    original(self);
}
static bool Arrow_tick_Entity_hurt_injection(Entity *self, __attribute__((unused)) Entity *cause, int damage) {
    on_projectile_hit(current_shooter, self);
    // Call Original Method
    return self->hurt(cause, damage);
}
static int Arrow_tick_Level_getTile_injection(Level *self, int x, int y, int z) {
    on_projectile_hit(current_shooter, x, y, z, nullptr);
    // Call Original Method
    return self->getTile(x, y, z);
}

// Throwable Hits
static void Throwable_tick_Throwable_onHit_injection(Throwable *self, HitResult *res) {
    if (res != nullptr && res->type != 2) {
        Entity *thrower = self->level->getEntity(self->getAuxData());
        if (res->type == 1 && res->entity) {
            on_projectile_hit(thrower, res->entity);
        } else {
            on_projectile_hit(thrower, res->x, res->y, res->z, nullptr);
        }
    }
    // Call Original Method
    self->onHit(res);
}

// Chat Events
static void Gui_addMessage_injection(Gui_addMessage_t original, Gui *gui, const std::string &text) {
    static bool recursing = false;
    if (recursing) {
        original(gui, text);
    } else {
        if (!chat_is_sending()) {
            api_add_chat_event(nullptr, text);
        }
        recursing = true;
        original(gui, text);
        recursing = false;
    }
}
bool api_suppress_chat_events = false;
static bool enabled = false;
void api_add_chat_event(const Player *sender, const std::string &message) {
    if (!enabled) {
        // Extended API Is Disabled
        return;
    }
    if (!sender && api_compat_mode) {
        // Non-Chat Message Are Only Supported In Non-Compatability Mode
        return;
    }
    if (api_suppress_chat_events) {
        // Do Not Record Messages Sent By The API
        return;
    }
    push_event(&ExtraClientData::chat_events, {
        message,
        sender ? sender->id : no_entity_id
    });
}

// Block Hit Events
static bool GameMode_useItemOn_injection(GameMode_useItemOn_t original, GameMode *self, Player *player, Level *level, ItemInstance *item, const int x, const int y, const int z, const int param_1, const Vec3 &param_2) {
    // Add Event
    if (item && item->id == Item::sword_iron->id) {
        push_event(&ExtraClientData::block_hit_events, {
            .owner_id = player->id,
            .x = x,
            .y = y,
            .z = z,
            .face = param_1
        });
    }
    // Call Original Method
    return original(self, player, level, item, x, y, z, param_1, param_2);
}
static bool CreatorMode_useItemOn_injection(__attribute__((unused)) CreatorMode_useItemOn_t original, CreatorMode *self, Player *player, Level *level, ItemInstance *item, const int x, const int y, const int z, const int param_1, const Vec3 &param_2) {
    return GameMode_useItemOn->get(false)((GameMode *) self, player, level, item, x, y, z, param_1, param_2);
}

// Init
void _init_api_events() {
    enabled = true;
    // Track Projectile Hits
    overwrite_calls(Arrow_tick, Arrow_tick_injection);
    overwrite_call((void *) 0x8b28c, Entity_hurt, Arrow_tick_Entity_hurt_injection);
    overwrite_call((void *) 0x8b388, Level_getTile, Arrow_tick_Level_getTile_injection);
    overwrite_call((void *) 0x8c5a4, Throwable_onHit, Throwable_tick_Throwable_onHit_injection);
    // Track GUI Messages
    overwrite_calls(Gui_addMessage, Gui_addMessage_injection);
    // Track Connected Clients
    overwrite_call((void *) 0x6bd78, CommandServer_setSocketBlocking, CommandServer__updateAccept_setSocketBlocking_injection);
    // Track Block Hits
    overwrite_calls(GameMode_useItemOn, GameMode_useItemOn_injection);
    overwrite_calls(CreatorMode_useItemOn, CreatorMode_useItemOn_injection);
}

// Handle Commands
std::string api_handle_event_command(CommandServer *server, const ConnectedClient &client, const std::string_view &cmd, const std::optional<int> id) {
    if (cmd == "clear") {
        // Clear Events
        if (id.has_value()) {
            clear_events(client, id.value());
        } else {
            clear_all_events(client);
        }
        return CommandServer::NullString;
    } else if (cmd == "chat.posts") {
        // Chat Events
        return get_chat_events(server, client, id);
    } else if (cmd == "block.hits") {
        // Block Hit Events
        return get_block_hit_events(server, client, id);
    } else if (cmd == "projectile.hits") {
        // Projectile Events
        return get_projectile_events(server, client, id);
    } else {
        // Invalid Command
        return CommandServer::Fail;
    }
}