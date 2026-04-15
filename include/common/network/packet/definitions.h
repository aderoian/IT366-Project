#ifndef NETWORK_PACKET_DEFINITIONS_H
#define NETWORK_PACKET_DEFINITIONS_H

#include "common/game/game.h"
#include "common/game/inventory.h"

#define PACKET_HEADER \
uint8_t packetID;  \
size_t length;

#define PACKET_HEADER_SIZE (sizeof(uint8_t) + sizeof(size_t))

#define BATCH_PACKET_ID 255

/**
 * @brief Generated packet ID table.
 */
typedef enum {
    PACKET_C2S_PLAYER_JOIN_REQUEST,
    PACKET_S2C_PLAYER_JOIN_RESPONSE,
    PACKET_C2S_PLAYER_INPUT_SNAPSHOT,
    PACKET_S2C_PLAYER_STATE_SNAPSHOT,
    PACKET_S2C_PLAYER_CREATE,
    PACKET_C2S_TOWER_REQUEST,
    PACKET_S2C_TOWER_SNAPSHOT,
    PACKET_S2C_INVENTORY_UPDATE,
    PACKET_S2C_GAME_STATE_SNAPSHOT,
    PACKET_S2C_ENEMY_SNAPSHOT,
    PACKET_COUNT
} packet_id_t;

typedef struct c2s_player_join_request_packet_s {
    PACKET_HEADER
    char name[16];
} c2s_player_join_request_packet_t;

typedef struct s2c_player_join_response_packet_s {
    PACKET_HEADER
    uint8_t success;
    uint32_t playerID;
    int64_t entityID;
    int32_t worldL;
    int32_t worldW;
    float spawnX;
    float spawnY;
    uint8_t teamID;
    game_state_t initialGameState;
} s2c_player_join_response_packet_t;

#define MOUSE_BUTTON_LEFT 1
#define MOUSE_BUTTON_RIGHT 2

typedef struct player_input_command_s {
    uint64_t tickNumber;
    int8_t axisX;
    int8_t axisY;
    uint8_t attack;
    float rotation;
} player_input_command_t;

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

typedef enum tower_request_id_e {
    TOWER_REQUEST_BUILD,
    TOWER_REQUEST_UPGRADE,
    TOWER_REQUEST_SELL,
    TOWER_REQUEST_SET_PRODUCTION_ENEMY
} tower_request_id_t;

typedef union tower_request_data_u {
    struct {
        float xPos;
        float yPos;
        uint32_t towerDefIndex;
    } buildData;
    struct {
        uint32_t towerID;
    } upgradeData;
    struct {
        uint32_t towerID;
    } sellData;
    struct {
        uint32_t towerID;
        uint32_t enemyDefIndex;
    } setProductionData;
} tower_request_data_t;

typedef struct c2s_tower_request_packet_s {
    PACKET_HEADER
    tower_request_id_t requestID;
    tower_request_data_t requestData;
} c2s_tower_request_packet_t;

typedef enum tower_snapshot_id_e {
    TOWER_SNAPSHOT_CREATE,
    TOWER_SNAPSHOT_SHOOT,
    TOWER_SNAPSHOT_UPGRADE,
    TOWER_SNAPSHOT_UPDATE,
    TOWER_SNAPSHOT_DESTROY
} tower_snapshot_id_t;

typedef union tower_snapshot_data_u {
    struct {
        float xPos;
        float yPos;
        uint32_t towerDefIndex;
        uint32_t towerID;
        int64_t entityID;
        uint32_t ownerPlayerID;
        uint8_t teamID;
        int32_t selectedEnemyDefIndex;
    } createData;
    struct {
        float xDir;
        float yDir;
    } shootData;
    struct {
        int level;
    } upgradeData;
    struct {
        float health;
        int32_t selectedEnemyDefIndex;
    } updateData;
} tower_snapshot_data_t;

typedef struct s2c_tower_snapshot_packet_s {
    PACKET_HEADER
    uint32_t towerID;
    tower_snapshot_id_t snapshotID;
    tower_snapshot_data_t snapshotData;
} s2c_tower_snapshot_packet_t;

typedef struct s2c_inventory_update_packet_s {
    PACKET_HEADER
    uint32_t playerID;
    inventory_transaction_t transaction;
} s2c_inventory_update_packet_t;

typedef struct s2c_game_state_snapshot_packet_s {
    PACKET_HEADER
    game_state_t gameState;
} s2c_game_state_snapshot_packet_t;

typedef enum {
    ENEMY_EVENT_SPAWN,
    ENEMY_EVENT_DESPAWN,
    ENEMY_EVENT_UPDATE
} enemy_event_id_t;

typedef union enemy_snapshot_data_u {
    struct {
        uint32_t enemyDefIndex;
        float xPos;
        float yPos;
        float rotation;
    } spawnData;
    struct {
        float xPos;
        float yPos;
        float rotation;
        float health;
        uint8_t attack;
    } updateData;
} enemy_snapshot_data_t;

typedef struct s2c_enemy_snapshot_packet_s {
    PACKET_HEADER
    int64_t enemyID;
    uint32_t eventID;
    enemy_snapshot_data_t eventData;
} s2c_enemy_snapshot_packet_t;

#endif /* NETWORK_PACKET_DEFINITIONS_H */