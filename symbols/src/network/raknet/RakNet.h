typedef enum {
    RAKNET_STARTED = 0,
    RAKNET_ALREADY_STARTED,
    INVALID_SOCKET_DESCRIPTORS,
    INVALID_MAX_CONNECTIONS,
    SOCKET_FAMILY_NOT_SUPPORTED,
    SOCKET_PORT_ALREADY_IN_USE,
    SOCKET_FAILED_TO_BIND,
    SOCKET_FAILED_TEST_SEND,
    PORT_CANNOT_BE_ZERO,
    FAILED_TO_CREATE_NETWORK_THREAD,
    COULD_NOT_GENERATE_GUID,
    STARTUP_OTHER_FAILURE
} RakNet_StartupResult;
typedef enum {
    IMMEDIATE_PRIORITY = 0,
    HIGH_PRIORITY,
    MEDIUM_PRIORITY,
    LOW_PRIORITY
} RakNet_PacketPriority;
typedef enum {
    UNRELIABLE,
    UNRELIABLE_SEQUENCED,
    RELIABLE,
    RELIABLE_ORDERED,
    RELIABLE_SEQUENCED,
    UNRELIABLE_WITH_ACK_RECEIPT,
    RELIABLE_WITH_ACK_RECEIPT,
    RELIABLE_ORDERED_WITH_ACK_RECEIPT
} RakNet_PacketReliability;