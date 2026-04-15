#include "common/logger.h"
#include "common/game/game.h"
#include "common/game/player.h"
#include "common/game/tower.h"
#include "common/game/enemy.h"
#include "../../../include/common/game/world/world.h"
#include "common/network/packet/handler.h"
#include "../../../include/server/network/server_network.h"
#include "server/server.h"
#include "server/game/player_manager.h"
#include "server/network/network_session.h"

static uint8_t player_can_modify_tower(const player_t *player, const entity_t *towerEntity) {
    const tower_state_t *tower;
    if (!player || !towerEntity || !towerEntity->data) {
        return 0;
    }
    if (g_game.state.mode != GAME_MODE_VERSUS) {
        return 1;
    }

    tower = (const tower_state_t *)towerEntity->data;
    return player->teamID != TEAM_NONE && player->teamID == tower->teamID;
}

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
        player->entity->id,
        g_game.world->size.x,
        g_game.world->size.y,
        player->position.x,
        player->position.y,
        player->teamID,
        &g_game.state
    );
    server_send_packet(&g_server, player, &packet, NET_UDP_FLAG_RELIABLE);
}

void handle_c2s_tower_request(const c2s_tower_request_packet_t *pkt, void *peer) {
    player_t *player;
    network_session_t *session;
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

    if (pkt->requestID == TOWER_REQUEST_BUILD) {
        towerDef = tower_def_get_by_index(g_game.towerManager, pkt->requestData.buildData.towerDefIndex);
        if (!towerDef) {
            log_warn("Received tower build request with invalid tower definition index %u from player ID %u", pkt->requestData.buildData.towerDefIndex, player->id);
            return;
        }

        if (!player_try_build_tower(player, towerDef, gfc_vector2d(pkt->requestData.buildData.xPos, pkt->requestData.buildData.yPos))) {
            log_info("Player ID %u failed to build tower at position (%f, %f) with definition index %u", player->id, pkt->requestData.buildData.xPos, pkt->requestData.buildData.yPos, pkt->requestData.buildData.towerDefIndex);
        } else {
            log_info("Player ID %u successfully built tower at position (%f, %f) with definition index %u", player->id, pkt->requestData.buildData.xPos, pkt->requestData.buildData.yPos, pkt->requestData.buildData.towerDefIndex);
        }
    } else if (pkt->requestID == TOWER_REQUEST_UPGRADE) {
        entity_t *tower = tower_get_by_id(g_game.towerManager, pkt->requestData.upgradeData.towerID);
        if (!tower) {
            log_warn("Received tower upgrade request for non-existent tower ID %u from player ID %u", pkt->requestData.upgradeData.towerID, player->id);
            return;
        }
        if (!player_can_modify_tower(player, tower)) {
            log_warn("Player ID %u attempted to upgrade opponent tower ID %u", player->id, pkt->requestData.upgradeData.towerID);
            return;
        }

        tower_try_upgrade(g_game.entityManager, g_game.towerManager, player, tower);
    } else if (pkt->requestID == TOWER_REQUEST_SELL) {
        entity_t *tower = tower_get_by_id(g_game.towerManager, pkt->requestData.sellData.towerID);
        if (!tower || !tower->data) {
            return;
        }
        if (!player_can_modify_tower(player, tower)) {
            return;
        }
        tower->think = entity_free;
    } else if (pkt->requestID == TOWER_REQUEST_SET_PRODUCTION_ENEMY) {
        entity_t *tower = tower_get_by_id(g_game.towerManager, pkt->requestData.setProductionData.towerID);
        tower_state_t *towerState;
        if (!tower || !tower->data) {
            return;
        }
        if (!player_can_modify_tower(player, tower)) {
            return;
        }
        towerState = (tower_state_t *)tower->data;
        if (towerState->def->type != TOWER_TYPE_UNIT_PRODUCTION) {
            return;
        }
        if (!enemy_def_get_by_index(g_game.enemyManager, (int)pkt->requestData.setProductionData.enemyDefIndex)) {
            return;
        }
        towerState->selectedEnemyDefIndex = (int)pkt->requestData.setProductionData.enemyDefIndex;
        towerState->dirtyFlags |= TOWER_DIRTY_SELECTION;
    }
}