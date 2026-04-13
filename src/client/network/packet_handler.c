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

        g_game.state = pkt->initialGameState;
        g_game.world = world_create_from_file(g_game.state.world);
        g_client.player = player_create(pkt->playerID, "Player"); // FIXME: Use actual player name (needs array serialization)
        g_client.player->position = gfc_vector2d(pkt->spawnX, pkt->spawnY);
        inventory_init(&g_client.player->inventory, 32);
        ent = player_entity_spawn(g_game.entityManager, g_client.player, g_client.player->position, "images/pointer.png");
        entity_set_id(g_game.entityManager, ent, pkt->entityID);
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
        const tower_def_t *def = tower_def_get_by_index(g_game.towerManager, pkt->snapshotData.createData.towerDefIndex);
        if (!def) {
            log_error("Received tower create packet with invalid tower definition index: %u", pkt->snapshotData.createData.towerDefIndex);
            return;
        }

        entity_t *tower = tower_place(g_game.entityManager, g_game.towerManager, def, gfc_vector2d(pkt->snapshotData.createData.xPos, pkt->snapshotData.createData.yPos), pkt->towerID);
        if (!tower) {
            log_error("Failed to create tower from server packet");
            return;
        }

        entity_set_id(g_game.entityManager, tower, pkt->snapshotData.createData.entityID);

        log_info("Tower created with ID: %u at position (%f, %f)", pkt->towerID, pkt->snapshotData.createData.xPos, pkt->snapshotData.createData.yPos);
    } else if (pkt->snapshotID == TOWER_SNAPSHOT_SHOOT) {
        entity_t *entity = tower_get_by_id(g_game.towerManager, pkt->towerID);
        tower_state_t *tower = (tower_state_t *)entity->data;
        tower->shootDirection = gfc_vector2d(pkt->snapshotData.shootData.xDir, pkt->snapshotData.shootData.yDir);
        tower_shoot_all(g_game.entityManager, entity);
    } else if (pkt->snapshotID == TOWER_SNAPSHOT_UPGRADE) {
        entity_t *entity = tower_get_by_id(g_game.towerManager, pkt->towerID);
        if (!entity) {
            log_error("Received tower change packet for non-existent tower ID: %u", pkt->towerID);
            return;
        }
        tower_upgrade(g_game.entityManager, g_game.towerManager, entity, pkt->snapshotData.upgradeData.level);
    } else if (pkt->snapshotID == TOWER_SNAPSHOT_UPDATE) {
        entity_t *entity = tower_get_by_id(g_game.towerManager, pkt->towerID);
        if (!entity) {
            log_error("Received tower update packet for non-existent tower ID: %u", pkt->towerID);
            return;
        }
        tower_state_t *tower = (tower_state_t *)entity->data;
        tower->health = pkt->snapshotData.updateData.health;
    } else if (pkt->snapshotID == TOWER_SNAPSHOT_DESTROY) {
        entity_t *entity = tower_get_by_id(g_game.towerManager, pkt->towerID);
        entity_free(g_game.entityManager, entity);
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
        entity_t * ent = enemy_spawn(g_game.entityManager, enemy_def_get_by_index(g_game.enemyManager, pkt->eventData.spawnData.enemyDefIndex), gfc_vector2d(pkt->eventData.spawnData.xPos, pkt->eventData.spawnData.yPos));
        entity_set_id(g_game.entityManager, ent, pkt->enemyID);
    } else if (pkt->eventID == ENEMY_EVENT_UPDATE) {
        entity_t *enemy = entity_get(g_game.entityManager, pkt->enemyID);
        if (!enemy) {
            log_error("Received move event for non-existent enemy ID: %lld", pkt->enemyID);
            return;
        }

        enemy->position.x = pkt->eventData.updateData.xPos;
        enemy->position.y = pkt->eventData.updateData.yPos;
        enemy->rotation = pkt->eventData.updateData.rotation;

        enemy_state_t *state = (enemy_state_t *)enemy->data;
        state->health = pkt->eventData.updateData.health;

        if (pkt->eventData.updateData.attack) {
            state->attackCooldownTimer = state->def->attackCooldown; // Reset attack cooldown on attack event
        }
    } else if (pkt->eventID == ENEMY_EVENT_DESPAWN) {
        entity_t *enemy = entity_get(g_game.entityManager, pkt->enemyID);
        if (!enemy) {
            log_error("Received despawn event for non-existent enemy ID: %lld", pkt->enemyID);
            return;
        }

        enemy->think = entity_free; // Mark enemy for removal
    } else {
        log_warn("Unknown enemy event ID: %u for enemy ID: %lld", pkt->eventID, pkt->enemyID);
    }
}