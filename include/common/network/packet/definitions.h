#ifndef NETWORK_PACKET_DEFINITIONS_H
#define NETWORK_PACKET_DEFINITIONS_H

#include <stdint.h>

/**
 * @brief List of all packets.
 */
#define PACKET_LIST(X) \
X(c2s_player_input,    C2S_PLAYER_INPUT,    PLAYER_INPUT_FIELDS)

/**
 * @brief Fields for each packet.
 */
#define PLAYER_INPUT_FIELDS(F) \
F(uint64_t, clientTick) \
F(uint64_t, lastServerTick) \
F(uint32_t,    axisX) \
F(uint32_t,    axisY)

/**
 * @brief Generated packet structures.
 */
#define FIELD(type, name) type name;

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