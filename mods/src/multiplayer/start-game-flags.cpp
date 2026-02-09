#include "internal.h"

#include <climits>
#include <optional>

#include <libreborn/patch.h>
#include <libreborn/util/string.h>

#include <symbols/StartGamePacket.h>
#include <symbols/RakNet_BitStream.h>

#include <mods/multiplayer/packets.h>

// Convert A Flag Index Into A Mask
static constexpr int flag_to_mask(const int index) {
    return 1 << index;
}

// Log Flags
static void log(const char *mode, const int flags) {
    constexpr int max = sizeof(flags) * CHAR_BIT;
    std::string str;
    for (int i = 0; i < max; i++) {
        const int mask = flag_to_mask(i);
        if ((flags & mask) != 0) {
            if (!str.empty()) {
                str += ", ";
            }
            str += safe_to_string(i);
        }
    }
    DEBUG("%s Extra Game Flags: [%s]", mode, str.c_str());
}

// Receive Flags From Server
static std::optional<int> received_flags;
static void StartGamePacket_read_injection(StartGamePacket_read_t original, StartGamePacket *self, RakNet_BitStream *stream) {
    // Call Original Method
    original(self, stream);
    // Read Flags
    int x;
    const bool success = stream->Read_int(&x);
    received_flags = success ? x : 0;
    log("Received", received_flags.value());
}
static constexpr bool has_flag(const int flags, const int flag) {
    const int mask = flag_to_mask(flag);
    return (flags & mask) != 0;
}
bool StartGameFlags::has_received_from_server(const int flag) {
    return has_flag(received_flags.value(), flag);
}

// Send Flags To Clients
static int host_flags = 0; // All Disabled By Default
static void StartGamePacket_write_injection(StartGamePacket_write_t original, StartGamePacket *self, RakNet_BitStream *stream) {
    // Call Original Method
    original(self, stream);
    // Send Flags
    if (host_flags != 0) {
        stream->Write_int(&host_flags);
    }
    log("Sent", host_flags);
}
void StartGameFlags::send_to_clients(const int flag) {
    const int mask = flag_to_mask(flag);
    host_flags |= mask;
}
bool StartGameFlags::will_send_to_clients(const int flag) {
    return has_flag(host_flags, flag);
}

// Init
void _init_multiplayer_start_game_flags() {
    // Receive Flags
    overwrite_calls(StartGamePacket_read, StartGamePacket_read_injection);
    // Send Flags
    overwrite_calls(StartGamePacket_write, StartGamePacket_write_injection);
}