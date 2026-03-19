#include "common/logger.h"
#include "common/game/game.h"
#include "common/game/player.h"
#include "common/game/tower.h"
#include "../../../include/common/game/world/world.h"
#include "common/network/packet/handler.h"
#include "../../../include/server/network/server_network.h"
#include "server/server.h"
#include "server/game/player_manager.h"
#include "server/network/network_session.h"

void handle_c2s_player_input_snapshot(const c2s_player_input_snapshot_packet_t *packet, void *peer) {
    player_t *player;
    network_session_t *session;
    if (!packet || !peer) {
        return;
    }

    session = (network_session_t *) ((net_udp_peer_t *) peer)->data;
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
        g_game.world->size.x,
        g_game.world->size.y,
        player->position.x,
        player->position.y
    ); // FIXME: Use actual spawn position
    server_send_packet(&g_server, player, &packet, NET_UDP_FLAG_RELIABLE);
}

void handle_c2s_tower_build_request(const c2s_tower_build_request_packet_t *pkt, void *peer) {
    player_t *player;
    network_session_t *session;
    entity_t *entity;
    tower_state_t *tower;
    s2c_tower_create_packet_t towerPkt;
    const tower_def_t *towerDef;
    if (!pkt || !peer) {
        return;
    }

    session = (network_session_t *) ((net_udp_peer_t *) peer)->data;
    if (!session) {
        log_warn("Received tower build request from peer without valid session");
        return;
    }

    player = player_manager_get(g_server.playerManager, session->sessionID);
    if (!player) {
        log_warn("Received tower build request for non-existent player with session ID %u", session->sessionID);
        return;
    }

    towerDef = tower_def_get_by_index(g_server.towerManager, pkt->towerDefIndex);
    if (!towerDef) {
        log_warn("Received tower build request with invalid tower definition index %u from player ID %u", pkt->towerDefIndex, player->id);
        return;
    }

    entity = tower_create_by_def(g_server.entityManager, g_server.towerManager, towerDef, (GFC_Vector2D){pkt->xPos, pkt->yPos});
    tower = (tower_state_t *) entity->data;
    if (entity) {
        log_info("Player ID %u built a tower at position (%f, %f) with definition index %u", player->id, pkt->xPos, pkt->yPos, pkt->towerDefIndex);
        create_s2c_tower_create(&towerPkt, pkt->xPos, pkt->yPos, pkt->towerDefIndex, tower->id);
        server_broadcast_packet(&g_server, &towerPkt, NET_UDP_FLAG_RELIABLE);
    } else {
        log_error("Failed to create tower for player ID %u at position (%f, %f) with definition index %u", player->id, pkt->xPos, pkt->yPos, pkt->towerDefIndex);
    }
}