#include "common/logger.h"
#include "common/game/game.h"
#include "common/game/player.h"
#include "common/game/world.h"
#include "common/network/packet/handler.h"
#include "server/server_network.h"
#include "server/server.h"
#include "server/game/player_manager.h"

void handle_c2s_player_input_snapshot(const c2s_player_input_snapshot_packet_t *packet, void *peer) {
    player_t *player;
    server_session_t *session;
    if (!packet || !peer) {
        return;
    }

    session = (server_session_t *) ((net_udp_peer_t *) peer)->data;
    if (!session) {
        log_warn("Received player input snapshot from peer without valid session");
        return;
    }

    player = player_manager_get(g_server.playerManager, session->sessionID);
    if (!player) {
        log_warn("Received player input snapshot for non-existent player with session ID %u", session->sessionID);
        return;
    }

    player_input_process(player, &packet->inputCommand, g_game.deltaTime);
}

void handle_c2s_player_join_request(const c2s_player_join_request_packet_t *pkt, void *peer) {
    player_t *player = server_create_player(&g_server, ((net_udp_peer_t *) peer)->data);

    s2c_player_join_response_packet_t packet;
    create_s2c_player_join_response(
        &packet,
        1,
        player->id,
        g_server.world->size.x,
        g_server.world->size.y,
        0,
        0
    ); // FIXME: Use actual spawn position
    server_send_packet(&g_server, player, PACKET_S2C_PLAYER_JOIN_RESPONSE, &packet, NET_UDP_FLAG_RELIABLE);
}

void handle_c2s_tower_build_request(const c2s_tower_build_request_packet_t *pkt, void *peer) {
    player_t *player;
    server_session_t *session;
    if (!pkt || !peer) {
        return;
    }

    session = (server_session_t *) ((net_udp_peer_t *) peer)->data;
    if (!session) {
        log_warn("Received tower build request from peer without valid session");
        return;
    }

    player = player_manager_get(g_server.playerManager, session->sessionID);
    if (!player) {
        log_warn("Received tower build request for non-existent player with session ID %u", session->sessionID);
        return;
    }

    log_info("Received tower build request from player %u for tower definition index %u at position (%f, %f)", player->id, pkt->towerDefIndex, pkt->xPos, pkt->yPos);
}