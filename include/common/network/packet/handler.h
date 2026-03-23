#ifndef NETWORK_PACKET_HANDLER_H
#define NETWORK_PACKET_HANDLER_H

#include "common/network/packet/io.h"

void handle_c2s_player_join_request(const c2s_player_join_request_packet_t *, void *);

void handle_s2c_player_join_response(const s2c_player_join_response_packet_t *, void *);

void handle_c2s_player_input_snapshot(const c2s_player_input_snapshot_packet_t *, void *);

void handle_s2c_player_state_snapshot(const s2c_player_state_snapshot_packet_t *, void *);

void handle_s2c_player_create(const s2c_player_create_packet_t *, void *);

void handle_c2s_tower_request(const c2s_tower_request_packet_t *, void *);

void handle_s2c_tower_snapshot(const s2c_tower_snapshot_packet_t *, void *);

void handle_s2c_inventory_update(const s2c_inventory_update_packet_t *, void *);

void handle_s2c_game_state_snapshot(const s2c_game_state_snapshot_packet_t *, void *);

void handle_s2c_enemy_snapshot(const s2c_enemy_snapshot_packet_t *, void *);

void receive_c2s_player_join_request(buffer_t buf, buffer_offset_t *off, void *c);

void receive_s2c_player_join_response(buffer_t buf, buffer_offset_t *off, void *c);

void receive_c2s_player_input_snapshot(buffer_t buf, buffer_offset_t *off, void *c);

void receive_s2c_player_state_snapshot(buffer_t buf, buffer_offset_t *off, void *c);

void receive_s2c_player_create(buffer_t buf, buffer_offset_t *off, void *c);

void receive_c2s_tower_request(buffer_t buf, buffer_offset_t *off, void *c);

void receive_s2c_tower_snapshot(buffer_t buf, buffer_offset_t *off, void *c);

void receive_s2c_inventory_update(buffer_t buf, buffer_offset_t *off, void *c);

void receive_s2c_game_state_snapshot(buffer_t buf, buffer_offset_t *off, void *c);

void receive_s2c_enemy_snapshot(buffer_t buf, buffer_offset_t *off, void *c);

void prepare_send_c2s_player_join_request(buffer_t buf, buffer_offset_t *off, void *c);

void prepare_send_s2c_player_join_response(buffer_t buf, buffer_offset_t *off, void *c);

void prepare_send_c2s_player_input_snapshot(buffer_t buf, buffer_offset_t *off, void *c);

void prepare_send_s2c_player_state_snapshot(buffer_t buf, buffer_offset_t *off, void *c);

void prepare_send_s2c_player_create(buffer_t buf, buffer_offset_t *off, void *c);

void prepare_send_c2s_tower_request(buffer_t buf, buffer_offset_t *off, void *c);

void prepare_send_s2c_tower_snapshot(buffer_t buf, buffer_offset_t *off, void *c);

void prepare_send_s2c_inventory_update(buffer_t buf, buffer_offset_t *off, void *c);

void prepare_send_s2c_game_state_snapshot(buffer_t buf, buffer_offset_t *off, void *c);

void prepare_send_s2c_enemy_snapshot(buffer_t buf, buffer_offset_t *off, void *c);

typedef void (*packet_receive_fn)(
    buffer_t buffer,
    buffer_offset_t *offset,
    void *context
);

extern packet_receive_fn packet_dispatch_table[PACKET_COUNT];

typedef void (*packet_send_fn) (
    buffer_t buffer,
    buffer_offset_t *offset,
    void *context
);

extern packet_send_fn packet_send_table[PACKET_COUNT];

#endif /* NETWORK_PACKET_HANDLER_H */