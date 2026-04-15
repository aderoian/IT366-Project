#ifndef NETWORK_PACKET_IO_H
#define NETWORK_PACKET_IO_H

#include <stddef.h>
#include <stdint.h>

#include "common/game/game.h"
#include "common/network/packet/definitions.h"

#define MAX_STRING_LENGTH UINT16_MAX
#define MAX_ITEMS_LENGTH UINT8_MAX

typedef uint8_t *buffer_t;
typedef size_t buffer_offset_t;

void write_uint8(buffer_t buffer, buffer_offset_t *offset, uint8_t value);
void write_uint16(buffer_t buffer, buffer_offset_t *offset, uint16_t value);
void write_uint32(buffer_t buffer, buffer_offset_t *offset, uint32_t value);
void write_uint64(buffer_t buffer, buffer_offset_t *offset, uint64_t value);
void write_int8(buffer_t buffer, buffer_offset_t *offset, int64_t value);
void write_int16(buffer_t buffer, buffer_offset_t *offset, int16_t value);
void write_int32(buffer_t buffer, buffer_offset_t *offset, int32_t value);
void write_int64(buffer_t buffer, buffer_offset_t *offset, int64_t value);
void write_float(buffer_t buffer, buffer_offset_t *offset, float value);
void write_double(buffer_t buffer, buffer_offset_t *offset, double value);
void write_string(buffer_t buffer, buffer_offset_t *offset, const char *str, uint16_t maxLen);

uint8_t read_uint8(buffer_t buffer, buffer_offset_t *offset);
uint16_t read_uint16(buffer_t buffer, buffer_offset_t *offset);
uint32_t read_uint32(buffer_t buffer, buffer_offset_t *offset);
uint64_t read_uint64(buffer_t buffer, buffer_offset_t *offset);
int8_t read_int8(buffer_t buffer, buffer_offset_t *offset);
int16_t read_int16(buffer_t buffer, buffer_offset_t *offset);
int32_t read_int32(buffer_t buffer, buffer_offset_t *offset);
int64_t read_int64(buffer_t buffer, buffer_offset_t *offset);
float read_float(buffer_t buffer, buffer_offset_t *offset);
double read_double(buffer_t buffer, buffer_offset_t *offset);
char *read_string(buffer_t buffer, buffer_offset_t *offset, char *out, uint16_t *outCount, uint16_t maxLen);

void write_game_state(buffer_t buffer, buffer_offset_t *offset, const game_state_t *state);
void write_player_input_command(buffer_t buffer, buffer_offset_t *offset, const player_input_command_t *cmd);
void write_item(buffer_t buffer, buffer_offset_t *offset, const item_t *item);
void write_item_array(buffer_t buffer, buffer_offset_t *offset, const item_t *items, size_t count);
void write_inventory_transaction(buffer_t buffer, buffer_offset_t *offset, const inventory_transaction_t *transaction);

void read_game_state(buffer_t buffer, buffer_offset_t *offset, game_state_t *state);
void read_player_input_command(buffer_t buffer, buffer_offset_t *offset, player_input_command_t *cmd);
void read_item(buffer_t buffer, buffer_offset_t *offset, item_t *transaction);
item_t *read_item_array(buffer_t buffer, buffer_offset_t *offset, uint16_t *outCount, uint16_t maxCount);
void read_inventory_transaction(buffer_t buffer, buffer_offset_t *offset, inventory_transaction_t *transaction);

void write_c2s_player_join_request(buffer_t, buffer_offset_t *, const c2s_player_join_request_packet_t *);

void read_c2s_player_join_request(buffer_t, buffer_offset_t *, c2s_player_join_request_packet_t *);

void create_c2s_player_join_request(c2s_player_join_request_packet_t *pkt, char *name);

void write_s2c_player_join_response(buffer_t, buffer_offset_t *, const s2c_player_join_response_packet_t *);

void read_s2c_player_join_response(buffer_t, buffer_offset_t *, s2c_player_join_response_packet_t *);

void create_s2c_player_join_response(s2c_player_join_response_packet_t *pkt, uint8_t success, uint32_t playerID, int64_t entityID,
                                     int32_t worldL, int32_t worldW, float spawnX, float spawnY, uint8_t teamID, game_state_t *initialGameState);

void write_c2s_player_input_snapshot(buffer_t, buffer_offset_t *, const c2s_player_input_snapshot_packet_t *);

void read_c2s_player_input_snapshot(buffer_t, buffer_offset_t *, c2s_player_input_snapshot_packet_t *);

void create_c2s_player_input_snapshot(c2s_player_input_snapshot_packet_t *pkt, player_input_command_t *inputCommand);

void write_s2c_player_state_snapshot(buffer_t, buffer_offset_t *, const s2c_player_state_snapshot_packet_t *);

void read_s2c_player_state_snapshot(buffer_t, buffer_offset_t *, s2c_player_state_snapshot_packet_t *);

void create_s2c_player_state_snapshot(s2c_player_state_snapshot_packet_t *pkt, uint64_t tickNumber,
                                      float xPos, float yPos);

void write_s2c_player_create(buffer_t, buffer_offset_t *, const s2c_player_create_packet_t *);

void read_s2c_player_create(buffer_t, buffer_offset_t *, s2c_player_create_packet_t *);

void create_s2c_player_create(s2c_player_create_packet_t *pkt, uint32_t playerID, float spawnX,
                              float spawnY);

void write_c2s_tower_request(buffer_t, buffer_offset_t *, const c2s_tower_request_packet_t *);

void read_c2s_tower_request(buffer_t, buffer_offset_t *, c2s_tower_request_packet_t *);

void create_c2s_tower_request(c2s_tower_request_packet_t *pkt, tower_request_id_t id, tower_request_data_t *data);

void write_s2c_tower_snapshot(buffer_t, buffer_offset_t *, const s2c_tower_snapshot_packet_t *);

void read_s2c_tower_snapshot(buffer_t, buffer_offset_t *, s2c_tower_snapshot_packet_t *);

void create_s2c_tower_snapshot(s2c_tower_snapshot_packet_t *pkt, uint32_t towerID, tower_snapshot_id_t snapshotID, tower_snapshot_data_t *eventData);

void write_s2c_inventory_update(buffer_t, buffer_offset_t *, const s2c_inventory_update_packet_t *);

void read_s2c_inventory_update(buffer_t, buffer_offset_t *, s2c_inventory_update_packet_t *);

void create_s2c_inventory_update(s2c_inventory_update_packet_t *pkt, uint32_t playerID,
                                 inventory_transaction_t *transaction);

void write_s2c_game_state_snapshot(buffer_t, buffer_offset_t *, const s2c_game_state_snapshot_packet_t *);

void read_s2c_game_state_snapshot(buffer_t, buffer_offset_t *, s2c_game_state_snapshot_packet_t *);

void create_s2c_game_state_snapshot(s2c_game_state_snapshot_packet_t *pkt, game_state_t *state);

void write_s2c_enemy_snapshot(buffer_t, buffer_offset_t *, const s2c_enemy_snapshot_packet_t *);

void read_s2c_enemy_snapshot(buffer_t, buffer_offset_t *, s2c_enemy_snapshot_packet_t *);

void create_s2c_enemy_snapshot(s2c_enemy_snapshot_packet_t *pkt, int64_t enemyID, uint32_t eventID, enemy_snapshot_data_t *eventData);

#endif /* NETWORK_PACKET_IO_H */