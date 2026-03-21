#ifndef NETWORK_PACKET_DEFINITIONS_H
#define NETWORK_PACKET_DEFINITIONS_H

#include "common/game/inventory.h"

#define PACKET_HEADER \
uint8_t packetID;  \
size_t length;

#define PACKET_HEADER_SIZE (sizeof(uint8_t) + sizeof(size_t))

#define BATCH_PACKET_ID 255

typedef struct player_input_command_s {
    uint64_t tickNumber;
    int32_t axisX;
    int32_t axisY;
} player_input_command_t;

/**
 * @brief Generated packet ID table.
 */
typedef enum {
    PACKET_C2S_PLAYER_JOIN_REQUEST,
    PACKET_S2C_PLAYER_JOIN_RESPONSE,
    PACKET_C2S_PLAYER_INPUT_SNAPSHOT,
    PACKET_S2C_PLAYER_STATE_SNAPSHOT,
    PACKET_S2C_PLAYER_CREATE,
    PACKET_C2S_TOWER_BUILD_REQUEST,
    PACKET_S2C_TOWER_CREATE,
    PACKET_S2C_TOWER_EVENT,
    PACKET_S2C_INVENTORY_UPDATE,
    PACKET_COUNT
} packet_id_t;

typedef struct c2s_player_join_request_packet_s {
    PACKET_HEADER
    char *name;
} c2s_player_join_request_packet_t;

typedef struct s2c_player_join_response_packet_s {
    PACKET_HEADER
    uint8_t success;
    uint32_t playerID;
    int32_t worldL;
    int32_t worldW;
    float spawnX;
    float spawnY;
} s2c_player_join_response_packet_t;

typedef struct c2s_player_input_snapshot_packet_s {
    PACKET_HEADER
    player_input_command_t inputCommand;
} c2s_player_input_snapshot_packet_t;

typedef struct s2c_player_state_snapshot_packet_s {
    PACKET_HEADER
    uint64_t tickNumber;
    float xPos;
    float yPos;
} s2c_player_state_snapshot_packet_t;

typedef struct s2c_player_create_packet_s {
    PACKET_HEADER
    uint32_t playerID;
    float spawnX;
    float spawnY;
} s2c_player_create_packet_t;

typedef struct c2s_tower_build_request_packet_s {
    PACKET_HEADER
    float xPos;
    float yPos;
    uint32_t towerDefIndex;
} c2s_tower_build_request_packet_t;

typedef struct s2c_tower_create_packet_s {
    PACKET_HEADER
    float xPos;
    float yPos;
    uint32_t towerDefIndex;
    uint32_t towerID;
} s2c_tower_create_packet_t;

typedef struct s2c_tower_event_packet_s {
    PACKET_HEADER
    uint32_t towerID;
    uint32_t eventID;
    uint64_t data;
} s2c_tower_event_packet_t;

typedef struct s2c_inventory_update_packet_s {
    PACKET_HEADER
    uint32_t playerID;
    inventory_transaction_t transaction;
} s2c_inventory_update_packet_t;

#endif /* NETWORK_PACKET_DEFINITIONS_H */