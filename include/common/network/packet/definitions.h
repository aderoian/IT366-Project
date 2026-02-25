#ifndef NETWORK_PACKET_DEFINITIONS_H
#define NETWORK_PACKET_DEFINITIONS_H

#include "common/types.h"

/**
 * @brief List of all packets.
 */
#define PACKET_LIST(X) \
X(c2s_player_join_request, C2S_PLAYER_JOIN_REQUEST, PLAYER_JOIN_REQUEST_FIELDS) \
X(s2c_player_join_response, S2C_PLAYER_JOIN_RESPONSE, PLAYER_JOIN_RESPONSE_FIELDS) \
X(c2s_player_input_snapshot, C2S_PLAYER_INPUT_SNAPSHOT, PLAYER_INPUT_SNAPSHOT_FIELDS) \
X(s2c_player_state_snapshot, S2C_PLAYER_STATE_SNAPSHOT, PLAYER_STATE_SNAPSHOT_FIELDS) \
X(s2c_player_create, S2C_PLAYER_CREATE, PLAYER_CREATE_PARAMS_FIELDS) \
X(c2s_tower_build_request, C2S_TOWER_BUILD_REQUEST, TOWER_BUILD_REQUEST_FIELDS) \
X(s2c_tower_create, S2C_TOWER_CREATE, TOWER_CREATE_FIELDS)

/**
 * @brief Fields for each packet.
 */

#define PLAYER_JOIN_REQUEST_FIELDS(F) \

#define PLAYER_JOIN_RESPONSE_FIELDS(F)  \
F(net_uint8_t,  success, PRIMITIVE)     \
F(net_uint32_t, playerID, PRIMITIVE)    \
F(net_int32_t,  worldL,   PRIMITIVE)    \
F(net_int32_t,  worldW,   PRIMITIVE)    \
F(net_float_t,  spawnX,   PRIMITIVE)    \
F(net_float_t,  spawnY,   PRIMITIVE)

#define PLAYER_INPUT_SNAPSHOT_FIELDS(F) \
F(player_input_command_t, inputCommand, CUSTOM) \

#define PLAYER_STATE_SNAPSHOT_FIELDS(F) \
F(net_uint64_t, tickNumber, PRIMITIVE) \
F(net_float_t, xPos, PRIMITIVE) \
F(net_float_t, yPos, PRIMITIVE)

#define PLAYER_CREATE_PARAMS_FIELDS(F) \
F(net_uint32_t, playerID, PRIMITIVE) \
F(net_float_t,  spawnX,   PRIMITIVE) \
F(net_float_t,  spawnY,   PRIMITIVE)

#define TOWER_BUILD_REQUEST_FIELDS(F) \
F(net_float_t, xPos, PRIMITIVE) \
F(net_float_t, yPos, PRIMITIVE) \
F(net_uint32_t, towerDefIndex, PRIMITIVE)

#define TOWER_CREATE_FIELDS(F) \
F(net_float_t, xPos, PRIMITIVE) \
F(net_float_t, yPos, PRIMITIVE) \
F(net_uint32_t, towerDefIndex, PRIMITIVE) \
F(net_uint32_t, towerID, PRIMITIVE)

/**
 * @brief Generated packet structures.
 */
#define FIELD(type, name, _) type name;

#define PACKET_STRUCT(name, id, fields) \
typedef struct name##_packet_s { \
net_int8_t packetID; /* Always first field */ \
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