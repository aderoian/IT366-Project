#include "common/logger.h"
#include "common/game/game.h"
#include "common/game/player.h"
#include "common/game/tower.h"
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
    tower_state_t *tower;
    s2c_tower_create_packet_t towerPkt;
    const tower_def_t *towerDef;
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

    towerDef = tower_def_get_by_index(pkt->towerDefIndex);
    if (!towerDef) {
        log_warn("Received tower build request with invalid tower definition index %u from player ID %u", pkt->towerDefIndex, player->id);
        return;
    }

    tower = tower_create_by_def(towerDef, (GFC_Vector2D){pkt->xPos, pkt->yPos});
    if (tower) {
        log_info("Player ID %u built a tower at position (%f, %f) with definition index %u", player->id, pkt->xPos, pkt->yPos, pkt->towerDefIndex);
        create_s2c_tower_create(&towerPkt, pkt->xPos, pkt->yPos, pkt->towerDefIndex, tower->id);
        server_broadcast_packet(&g_server, PACKET_S2C_TOWER_CREATE, &towerPkt, NET_UDP_FLAG_RELIABLE);
    } else {
        log_error("Failed to create tower for player ID %u at position (%f, %f) with definition index %u", player->id, pkt->xPos, pkt->yPos, pkt->towerDefIndex);
    }
}