#include "client/camera.h"
#include "common/logger.h"
#include "common/network/packet/handler.h"

#include "client/client.h"
#include "common/game/enemy.h"
#include "common/game/tower.h"

typedef struct remote_player_state_s {
    uint8_t inUse;
    uint32_t playerID;
    player_t *player;
} remote_player_state_t;

#define MAX_REMOTE_PLAYERS 256

static remote_player_state_t g_remotePlayers[MAX_REMOTE_PLAYERS] = {0};

static remote_player_state_t *remote_player_get(uint32_t playerID) {
    size_t i;
    for (i = 0; i < MAX_REMOTE_PLAYERS; ++i) {
        if (g_remotePlayers[i].inUse && g_remotePlayers[i].playerID == playerID) {
            return &g_remotePlayers[i];
        }
    }
    return NULL;
}

static remote_player_state_t *remote_player_add(uint32_t playerID, player_t *player) {
    size_t i;
    for (i = 0; i < MAX_REMOTE_PLAYERS; ++i) {
        if (!g_remotePlayers[i].inUse) {
            g_remotePlayers[i].inUse = 1;
            g_remotePlayers[i].playerID = playerID;
            g_remotePlayers[i].player = player;
            return &g_remotePlayers[i];
        }
    }
    return NULL;
}

static void remote_player_remove(uint32_t playerID) {
    remote_player_state_t *state = remote_player_get(playerID);
    if (!state) {
        return;
    }

    state->inUse = 0;
    state->playerID = 0;
    state->player = NULL;
}

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
        g_game.player = g_client.player;
        g_client.player->teamID = pkt->teamID;
        g_client.player->position = gfc_vector2d(pkt->spawnX, pkt->spawnY);
        inventory_init(&g_client.player->inventory, 32);
        ent = player_entity_spawn(g_game.entityManager, g_client.player, g_client.player->position, "images/pointer.png");
        entity_set_id(g_game.entityManager, ent, pkt->entityID);
        g_client.state = CLIENT_PLAYING;
        camera_set_target(&g_camera, ent);
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

void handle_s2c_player_state_update(const s2c_player_state_update_packet_t *pkt, void *client) {
    remote_player_state_t *state;
    entity_t *ent;

    if (!pkt) {
        return;
    }

    if (g_client.player && pkt->playerID == g_client.player->id) {
        // if (pkt->eventType == PLAYER_STATE_UPDATE_SYNC) {
        //     player_input_process_server(g_client.player, pkt->eventData.syncData.tickNumber, pkt->eventData.syncData.xPos, pkt->eventData.syncData.yPos);
        //     if (g_client.player->entity) {
        //         g_client.player->entity->rotation = pkt->eventData.syncData.rotation;
        //         if (pkt->eventData.syncData.attack) {
        //             g_client.player->attackCooldown = 0.5f;
        //         }
        //     }
        // }
        return;
    }

    if (pkt->eventType == PLAYER_STATE_UPDATE_CREATE) {
        log_info("Creating remote player with ID: %u at position (%f, %f)", pkt->playerID, pkt->eventData.createData.xPos, pkt->eventData.createData.yPos);
        if (!g_game.entityManager) {
            return;
        }
        if (remote_player_get(pkt->playerID)) {
            return;
        }

        player_t *player = player_create(pkt->playerID, "RemotePlayer");
        if (!player) {
            return;
        }
        player->teamID = pkt->eventData.createData.teamID;
        player->position = gfc_vector2d(pkt->eventData.createData.xPos, pkt->eventData.createData.yPos);
        ent = player_entity_spawn(g_game.entityManager, player, player->position, "images/pointer.png");
        if (!ent) {
            player_destroy(player);
            return;
        }
        entity_set_id(g_game.entityManager, ent, pkt->entityID);
        if (!remote_player_add(pkt->playerID, player)) {
            entity_free(g_game.entityManager, ent);
            return;
        }
        log_info("Remote player created with ID: %u at position (%f, %f)", pkt->playerID, pkt->eventData.createData.xPos, pkt->eventData.createData.yPos);
    } else if (pkt->eventType == PLAYER_STATE_UPDATE_SYNC) {
        state = remote_player_get(pkt->playerID);
        if (!state || !state->player || !state->player->entity) {
            return;
        }

        state->player->position = gfc_vector2d(pkt->eventData.syncData.xPos, pkt->eventData.syncData.yPos);
        state->player->entity->rotation = pkt->eventData.syncData.rotation;
        if (pkt->eventData.syncData.attack) {
            state->player->attackCooldown = 0.5f;
        }
    } else if (pkt->eventType == PLAYER_STATE_UPDATE_DELETE) {
        state = remote_player_get(pkt->playerID);
        if (!state || !state->player || !state->player->entity) {
            remote_player_remove(pkt->playerID);
            return;
        }

        entity_free(g_game.entityManager, state->player->entity);
        remote_player_remove(pkt->playerID);
    }
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
        tower_state_t *towerState = (tower_state_t *)tower->data;
        towerState->ownerPlayerID = pkt->snapshotData.createData.ownerPlayerID;
        towerState->teamID = pkt->snapshotData.createData.teamID;
        towerState->selectedEnemyDefIndex = pkt->snapshotData.createData.selectedEnemyDefIndex;

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
        tower->selectedEnemyDefIndex = pkt->snapshotData.updateData.selectedEnemyDefIndex;
    } else if (pkt->snapshotID == TOWER_SNAPSHOT_DESTROY) {
        entity_t *entity = tower_get_by_id(g_game.towerManager, pkt->towerID);
        if (entity) {
            entity_free(g_game.entityManager, entity);
        }
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
    if (g_game.state.mode == GAME_MODE_VERSUS && g_game.state.winnerTeamID != TEAM_NONE) {
        log_info("Versus match ended. Winner: Team %u", g_game.state.winnerTeamID);
    }
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