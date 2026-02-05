#include "common/logger.h"
#include "common/network/packet/handler.h"
#include "server/network.h"
#include "server/server.h"

void handle_c2s_player_input_snapshot(const c2s_player_input_snapshot_packet_t *packet, void *peer) {
    log_info("Received player input packet: %llu, %llu, %d, %d", packet->inputCommand.clientTick, packet->inputCommand.lastServerTick, packet->inputCommand.axisX, packet->inputCommand.axisY);
}

void handle_c2s_player_join_request(const c2s_player_join_request_packet_t *pkt, void *peer) {
    // TODO: Add player to a list to players and their IDs
    log_info("Received player join request");
    s2c_player_join_response_packet_t packet;
    create_s2c_player_join_response(&packet, 1, 5);
    server_network_send(g_server.network, peer, PACKET_S2C_PLAYER_JOIN_RESPONSE, &packet);
}