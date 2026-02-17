#include "common/logger.h"
#include "common/game/player.h"
#include "common/network/packet/handler.h"
#include "server/server_network.h"
#include "server/server.h"

void handle_c2s_player_input_snapshot(const c2s_player_input_snapshot_packet_t *packet, void *peer) {

}

void handle_c2s_player_join_request(const c2s_player_join_request_packet_t *pkt, void *peer) {
    player_t *player = server_create_player(&g_server, ((net_udp_peer_t *) peer)->data);

    s2c_player_join_response_packet_t packet;
    create_s2c_player_join_response(&packet, 1, player->id, 0, 0); // FIXME: Use actual spawn position
    server_send_packet(&g_server, player, PACKET_S2C_PLAYER_JOIN_RESPONSE, &packet, NET_UDP_FLAG_RELIABLE);
}