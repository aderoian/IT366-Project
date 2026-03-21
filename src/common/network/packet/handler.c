#include "common/network/packet/handler.h"

void receive_c2s_player_join_request(buffer_t buf, buffer_offset_t *off, void *c) {
    c2s_player_join_request_packet_t pkt;
    read_c2s_player_join_request(buf, off, &pkt);
    handle_c2s_player_join_request(&pkt, c);
}

void receive_s2c_player_join_response(buffer_t buf, buffer_offset_t *off, void *c) {
    s2c_player_join_response_packet_t pkt;
    read_s2c_player_join_response(buf, off, &pkt);
    handle_s2c_player_join_response(&pkt, c);
}

void receive_c2s_player_input_snapshot(buffer_t buf, buffer_offset_t *off, void *c) {
    c2s_player_input_snapshot_packet_t pkt;
    read_c2s_player_input_snapshot(buf, off, &pkt);
    handle_c2s_player_input_snapshot(&pkt, c);
}

void receive_s2c_player_state_snapshot(buffer_t buf, buffer_offset_t *off, void *c) {
    s2c_player_state_snapshot_packet_t pkt;
    read_s2c_player_state_snapshot(buf, off, &pkt);
    handle_s2c_player_state_snapshot(&pkt, c);
}

void receive_s2c_player_create(buffer_t buf, buffer_offset_t *off, void *c) {
    s2c_player_create_packet_t pkt;
    read_s2c_player_create(buf, off, &pkt);
    handle_s2c_player_create(&pkt, c);
}

void receive_c2s_tower_build_request(buffer_t buf, buffer_offset_t *off, void *c) {
    c2s_tower_build_request_packet_t pkt;
    read_c2s_tower_build_request(buf, off, &pkt);
    handle_c2s_tower_build_request(&pkt, c);
}

void receive_s2c_tower_create(buffer_t buf, buffer_offset_t *off, void *c) {
    s2c_tower_create_packet_t pkt;
    read_s2c_tower_create(buf, off, &pkt);
    handle_s2c_tower_create(&pkt, c);
}

void receive_s2c_tower_event(buffer_t buf, buffer_offset_t *off, void *c) {
    s2c_tower_event_packet_t pkt;
    read_s2c_tower_event(buf, off, &pkt);
    handle_s2c_tower_event(&pkt, c);
}

void receive_s2c_inventory_update(buffer_t buf, buffer_offset_t *off, void *c) {
    s2c_inventory_update_packet_t pkt;
    read_s2c_inventory_update(buf, off, &pkt);
    handle_s2c_inventory_update(&pkt, c);
}

void receive_s2c_game_state_snapshot(buffer_t buf, buffer_offset_t *off, void *c) {
    s2c_game_state_snapshot_packet_t pkt;
    read_s2c_game_state_snapshot(buf, off, &pkt);
    handle_s2c_game_state_snapshot(&pkt, c);
}

packet_receive_fn packet_dispatch_table[PACKET_COUNT] = {
    [PACKET_C2S_PLAYER_JOIN_REQUEST] = receive_c2s_player_join_request,
    [PACKET_S2C_PLAYER_JOIN_RESPONSE] = receive_s2c_player_join_response,
    [PACKET_C2S_PLAYER_INPUT_SNAPSHOT] = receive_c2s_player_input_snapshot,
    [PACKET_S2C_PLAYER_STATE_SNAPSHOT] = receive_s2c_player_state_snapshot,
    [PACKET_S2C_PLAYER_CREATE] = receive_s2c_player_create,
    [PACKET_C2S_TOWER_BUILD_REQUEST] = receive_c2s_tower_build_request,
    [PACKET_S2C_TOWER_CREATE] = receive_s2c_tower_create,
    [PACKET_S2C_TOWER_EVENT] = receive_s2c_tower_event,
    [PACKET_S2C_INVENTORY_UPDATE] = receive_s2c_inventory_update,
    [PACKET_S2C_GAME_STATE_SNAPSHOT] = receive_s2c_game_state_snapshot,
};

void prepare_send_c2s_player_join_request(buffer_t buf, buffer_offset_t *off, void *c) {
    write_c2s_player_join_request(buf, off, (c2s_player_join_request_packet_t *) c);
}

void prepare_send_s2c_player_join_response(buffer_t buf, buffer_offset_t *off, void *c) {
    write_s2c_player_join_response(buf, off, (s2c_player_join_response_packet_t *) c);
}

void prepare_send_c2s_player_input_snapshot(buffer_t buf, buffer_offset_t *off, void *c) {
    write_c2s_player_input_snapshot(buf, off, (c2s_player_input_snapshot_packet_t *) c);
}

void prepare_send_s2c_player_state_snapshot(buffer_t buf, buffer_offset_t *off, void *c) {
    write_s2c_player_state_snapshot(buf, off, (s2c_player_state_snapshot_packet_t *) c);
}

void prepare_send_s2c_player_create(buffer_t buf, buffer_offset_t *off, void *c) {
    write_s2c_player_create(buf, off, (s2c_player_create_packet_t *) c);
}

void prepare_send_c2s_tower_build_request(buffer_t buf, buffer_offset_t *off, void *c) {
    write_c2s_tower_build_request(buf, off, (c2s_tower_build_request_packet_t *) c);
}

void prepare_send_s2c_tower_create(buffer_t buf, buffer_offset_t *off, void *c) {
    write_s2c_tower_create(buf, off, (s2c_tower_create_packet_t *) c);
}

void prepare_send_s2c_tower_event(buffer_t buf, buffer_offset_t *off, void *c) {
    write_s2c_tower_event(buf, off, (s2c_tower_event_packet_t *) c);
}

void prepare_send_s2c_inventory_update(buffer_t buf, buffer_offset_t *off, void *c) {
    write_s2c_inventory_update(buf, off, (s2c_inventory_update_packet_t *) c);
}

void prepare_send_s2c_game_state_snapshot(buffer_t buf, buffer_offset_t *off, void *c) {
    write_s2c_game_state_snapshot(buf, off, (s2c_game_state_snapshot_packet_t *) c);
}

packet_send_fn packet_send_table[PACKET_COUNT] = {
    [PACKET_C2S_PLAYER_JOIN_REQUEST] = prepare_send_c2s_player_join_request,
    [PACKET_S2C_PLAYER_JOIN_RESPONSE] = prepare_send_s2c_player_join_response,
    [PACKET_C2S_PLAYER_INPUT_SNAPSHOT] = prepare_send_c2s_player_input_snapshot,
    [PACKET_S2C_PLAYER_STATE_SNAPSHOT] = prepare_send_s2c_player_state_snapshot,
    [PACKET_S2C_PLAYER_CREATE] = prepare_send_s2c_player_create,
    [PACKET_C2S_TOWER_BUILD_REQUEST] = prepare_send_c2s_tower_build_request,
    [PACKET_S2C_TOWER_CREATE] = prepare_send_s2c_tower_create,
    [PACKET_S2C_TOWER_EVENT] = prepare_send_s2c_tower_event,
    [PACKET_S2C_INVENTORY_UPDATE] = prepare_send_s2c_inventory_update,
    [PACKET_S2C_GAME_STATE_SNAPSHOT] = prepare_send_s2c_game_state_snapshot,
};
