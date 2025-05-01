#include <symbols/minecraft.h>

#include <libreborn/config.h>
#include <libreborn/patch.h>

#include <mods/feature/feature.h>

#include "internal.h"

// Fix Bug Where RakNetInstance Starts Pinging Potential Servers Before The "Join Game" Screen Is Opened
static RakNetInstance *RakNetInstance_injection(RakNetInstance_constructor_t original, RakNetInstance *rak_net_instance) {
    // Call Original Method
    RakNetInstance *result = original(rak_net_instance);
    // Fix
    rak_net_instance->pinging_for_hosts = false;
    // Return
    return result;
}

// Fix RakNet::RakString Security Bug
//
// RakNet::RakString's format constructor is often given unsanitized user input and is never used for formatting;
// this is a massive security risk, allowing clients to run arbitrary format specifiers, this disables the
// formatting functionality.
typedef RakNet_RakString *(*RakNet_RakString_constructor_2_t)(RakNet_RakString *self, const char *format, ...);
static RakNet_RakString_constructor_2_t RakNet_RakString_constructor_2 = (RakNet_RakString_constructor_2_t) 0xea5cc;
static RakNet_RakString *RakNet_RakString_injection(RakNet_RakString *rak_string, const char *format, ...) {
    // Call Original Method
    return RakNet_RakString_constructor_2(rak_string, "%s", format);
}

// Print Error Message If RakNet Startup Fails
static const char *RAKNET_ERROR_NAMES[] = {
    "Success",
    "Already Started",
    "Invalid Socket Descriptors",
    "Invalid Max Connections",
    "Socket Family Not Supported",
    "Part Already In Use",
    "Failed To Bind Port",
    "Failed Test Send",
    "Port Cannot Be 0",
    "Failed To Create Network Thread",
    "Unknown"
};
static RakNet_StartupResult RakNetInstance_host_RakNet_RakPeer_Startup_injection(RakNet_RakPeer *rak_peer, const unsigned short maxConnections, unsigned char *socketDescriptors, const uint socketDescriptorCount, const int threadPriority) {
    // Call Original Method
    const RakNet_StartupResult result = rak_peer->Startup(maxConnections, socketDescriptors, socketDescriptorCount, threadPriority);

    // Print Error
    if (result != RAKNET_STARTED) {
        CONDITIONAL_ERR(reborn_is_server(), "Failed To Start RakNet: %s", RAKNET_ERROR_NAMES[result]);
    }

    // Return
    return result;
}

// Init
void _init_multiplayer_raknet() {
    // Fix Bug Where RakNetInstance Starts Pinging Potential Servers Before The "Join Game" Screen Is Opened
    if (feature_has("Prevent Unnecessary Server Pinging", server_enabled)) {
        overwrite_calls(RakNetInstance_constructor, RakNetInstance_injection);
    }

    // Fix RakNet::RakString Security Bug
    if (feature_has("Patch RakNet Security Bug", server_enabled)) {
        overwrite_calls_manual((void *) RakNet_RakString_constructor_2, (void *) RakNet_RakString_injection);
    }

    // Print Error Message If RakNet Startup Fails
    if (feature_has("Log RakNet Startup Errors", server_enabled)) {
        overwrite_call((void *) 0x73778, RakNet_RakPeer_Startup, RakNetInstance_host_RakNet_RakPeer_Startup_injection);
    }
}