#include "common/logger.h"
#include "common/network/packet/handler.h"

#include "client/client.h"
#include "client/game/build.h"
#include "common/game/tower.h"

void handle_s2c_player_join_response(const s2c_player_join_response_packet_t *pkt, void *client) {
    if (!pkt) {
        return;
    }

    if (pkt->success) {
        if (g_client.state != CLIENT_JOINING) {
            log_warn("Received join response while not joining. Current state: %d", g_client.state);
            return;
        }

        log_info("Joined server successfully with Player ID: %u", pkt->playerID);

        g_client.player = player_create(pkt->playerID, "Player"); // FIXME: Use actual player name (needs array serialization)
        g_client.player->position = gfc_vector2d(pkt->spawnX, pkt->spawnY);
        player_entity_spawn(g_client.entityManager, g_client.player, g_client.player->position, "images/pointer.png");
        g_client.state = CLIENT_PLAYING;
        g_game.world = world_create(pkt->worldW, pkt->worldL, 0);

        build_mode_enter(tower_def_get(g_client.towerManager, "Arrow Tower"));
    } else {
        log_info("Failed to join server.");
    }
}

void handle_s2c_player_create(const s2c_player_create_packet_t *pkt, void *client) {
    log_info("Player created with ID: %u at position (%f, %f)", pkt->playerID, pkt->spawnX, pkt->spawnY);
}

void handle_s2c_player_state_snapshot(const s2c_player_state_snapshot_packet_t *pkt, void *client) {
    player_input_process_server(g_client.player, pkt->tickNumber, pkt->xPos, pkt->yPos);
}

void handle_s2c_tower_create(const s2c_tower_create_packet_t *pkt, void *client) {
    const tower_def_t *def = tower_def_get_by_index(g_client.towerManager, (int)pkt->towerDefIndex);
    if (!def) {
        log_error("Received tower create packet with invalid tower definition index: %u", pkt->towerDefIndex);
        return;
    }

    entity_t *tower = tower_place(g_client.entityManager, g_client.towerManager, def, gfc_vector2d(pkt->xPos, pkt->yPos), pkt->towerID);
    if (!tower) {
        log_error("Failed to create tower from server packet");
        return;
    }

    log_info("Tower created with ID: %u at position (%f, %f)", pkt->towerID, pkt->xPos, pkt->yPos);
}

void handle_s2c_tower_event(const s2c_tower_event_packet_t *pkt, void *client) {
        entity_t *entity = tower_get_by_id(g_client.towerManager, pkt->towerID);
        if (!entity) {
            log_error("Received tower event packet for non-existent tower ID: %u", pkt->towerID);
            return;
        }

        switch (pkt->eventID) {
            case TOWER_EVENT_SHOOT:
                tower_shoot_all(g_client.entityManager, entity);
            break;
        }
}