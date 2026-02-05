#ifndef NETWORK_PACKET_DEFINITIONS_H
#define NETWORK_PACKET_DEFINITIONS_H

#include <stdint.h>

#include "common/types.h"

/**
 * @brief List of all packets.
 */
#define PACKET_LIST(X) \
X(c2s_player_input_snapshot, C2S_PLAYER_INPUT_SNAPSHOT, PLAYER_INPUT_SNAPSHOT_FIELDS)

/**
 * @brief Fields for each packet.
 */
#define PLAYER_INPUT_SNAPSHOT_FIELDS(F) \
F(player_input_command, inputCommand, CUSTOM) \

/**
 * @brief Generated packet structures.
 */
#define FIELD(type, name, _) type##_t name;

#define PACKET_STRUCT(name, id, fields) \
typedef struct name##_packet_s { \
uint8_t packetID; /* Always first field */ \
fields(FIELD) \
} name##_packet_t;

PACKET_LIST(PACKET_STRUCT)
#undef PACKET_STRUCT
#undef FIELD

/**
 * @brief Generated packet ID table.
 */
typedef enum {
#define PACKET_ENUM(name, id, fields) PACKET_##id,
    PACKET_LIST(PACKET_ENUM)
#undef PACKET_ENUM
    PACKET_COUNT
} packet_id_t;

#endif /* NETWORK_PACKET_DEFINITIONS_H */