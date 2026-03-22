#include "common/logger.h"
#include "common/network/packet/handler.h"

#include "client/client.h"
#include "common/game/enemy.h"
#include "common/game/tower.h"

void handle_s2c_player_join_response(const s2c_player_join_response_packet_t *pkt, void *client) {
    entity_t *ent;
    if (!pkt) {
        return;
    }

    if (pkt->success) {
        if (g_client.state != CLIENT_JOINING) {
            log_warn("Received join response while not joining. Current state: %d", g_client.state);
            return;
        }

        log_info("Joined server successfully with Player ID: %u", pkt->playerID);

        g_game.world = world_create(pkt->worldW, pkt->worldL, 0);
        g_game.state = pkt->initialGameState;
        g_client.player = player_create(pkt->playerID, "Player"); // FIXME: Use actual player name (needs array serialization)
        g_client.player->position = gfc_vector2d(pkt->spawnX, pkt->spawnY);
        inventory_init(&g_client.player->inventory, 32);
        ent = player_entity_spawn(g_client.entityManager, g_client.player, g_client.player->position, "images/pointer.png");
        entity_set_id(g_client.entityManager, ent, pkt->entityID);
        g_client.state = CLIENT_PLAYING;
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

void handle_s2c_tower_snapshot(const s2c_tower_snapshot_packet_t *pkt, void *client) {
    if (pkt->snapshotID == TOWER_SNAPSHOT_CREATE) {
        const tower_def_t *def = tower_def_get_by_index(g_client.towerManager, pkt->snapshotData.createData.towerDefIndex);
        if (!def) {
            log_error("Received tower create packet with invalid tower definition index: %u", pkt->snapshotData.createData.towerDefIndex);
            return;
        }

        entity_t *tower = tower_place(g_client.entityManager, g_client.towerManager, def, gfc_vector2d(pkt->snapshotData.createData.xPos, pkt->snapshotData.createData.yPos), pkt->towerID);
        if (!tower) {
            log_error("Failed to create tower from server packet");
            return;
        }

        entity_set_id(g_client.entityManager, tower, pkt->snapshotData.createData.entityID);

        log_info("Tower created with ID: %u at position (%f, %f)", pkt->towerID, pkt->snapshotData.createData.xPos, pkt->snapshotData.createData.yPos);
    } else if (pkt->snapshotID == TOWER_SNAPSHOT_SHOOT) {
        entity_t *entity = tower_get_by_id(g_client.towerManager, pkt->towerID);
        tower_state_t *tower = (tower_state_t *)entity->data;
        tower->shootDirection = gfc_vector2d(pkt->snapshotData.shootData.xDir, pkt->snapshotData.shootData.yDir);
        tower_shoot_all(g_client.entityManager, entity);
    } else if (pkt->snapshotID == TOWER_SNAPSHOT_CHANGE) {
        // TODO: Implement tower change logic (e.g. level up)
    } else if (pkt->snapshotID == TOWER_SNAPSHOT_DESTROY) {
        entity_t *entity = tower_get_by_id(g_client.towerManager, pkt->towerID);
        entity_free(g_client.entityManager, entity);
    }
}


void handle_s2c_inventory_update(const s2c_inventory_update_packet_t *pkt, void *client) {
    if (!pkt) {
        return;
    }

    inventory_transaction_apply(&g_client.player->inventory, &pkt->transaction);
}

void handle_s2c_game_state_snapshot(const s2c_game_state_snapshot_packet_t *pkt, void *client) {
    if (!pkt) {
        return;
    }

    g_game.state = pkt->gameState;
}

void handle_s2c_enemy_snapshot(const s2c_enemy_snapshot_packet_t *pkt, void *client) {
    if (!pkt) {
        return;
    }

    if (pkt->eventID == ENEMY_EVENT_SPAWN) {
        entity_t * ent = enemy_spawn(g_client.entityManager, enemy_def_get_by_index(g_client.enemyManager, 0), gfc_vector2d(pkt->eventData.spawnData.xPos, pkt->eventData.spawnData.yPos));
        entity_set_id(g_client.entityManager, ent, pkt->enemyID);
    } else if (pkt->eventID == ENEMY_EVENT_MOVE) {
        entity_t *enemy = entity_get(g_client.entityManager, pkt->enemyID);
        if (!enemy) {
            log_error("Received move event for non-existent enemy ID: %lld", pkt->enemyID);
            return;
        }
        enemy->position.x = pkt->eventData.moveData.xPos;
        enemy->position.y = pkt->eventData.moveData.yPos;
        enemy->rotation = pkt->eventData.moveData.rotation;
    } else if (pkt->eventID == ENEMY_EVENT_ATTACK) {
        log_info("Enemy attack event for enemy ID: %lld", pkt->enemyID);
    } else if (pkt->eventID == ENEMY_EVENT_DESPAWN) {
        log_info("Enemy despawn event for enemy ID: %lld", pkt->enemyID);
    } else {
        log_info("Unknown enemy event ID: %u for enemy ID: %lld", pkt->eventID, pkt->enemyID);
    }
}