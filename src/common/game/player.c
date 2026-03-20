#include "gfc_input.h"
#include "gfc_types.h"

#include "common/logger.h"
#include "common/network/packet/definitions.h"
#include "common/network/packet/io.h"
#include "common/game/player.h"
#include "common/game/game.h"

#include "client/animation.h"
#include "client/camera.h"
#include "client/client.h"
#include "client/gf2d_sprite.h"
#include "common/game/tower.h"

#include "server/server.h"
#include "server/network/network_session.h"

#define MAX_DIVERSION 1.5f
#define MAX_TELEPORT_DISTANCE 5.0f

extern uint8_t __INF_RESOURCES;

typedef struct player_snapshot_s {
    GFC_Vector2D position;
    player_input_command_t cmd;
} player_snapshot_t;

void player_think(const entity_manager_t *entityManager, entity_t *ent);
void player_update(const entity_manager_t *entityManager, entity_t *ent, float deltaTime);
void player_draw(const entity_manager_t *entityManager, entity_t *ent);

player_t *player_create(uint32_t id, const char *name) {
    player_t *player = (player_t *)gfc_allocate_array(sizeof(player_t), 1);
    if (!player) {
        return NULL;
    }

    player->id = id;
    memcpy(player->name, name, sizeof(name) + 1);
    player->position = gfc_vector2d(0.0f, 0.0f);

    player->inputBuffer = (buf_spsc_ring_t *)malloc(sizeof(buf_spsc_ring_t));
    buf_spsc_ring_init(player->inputBuffer, 64, sizeof(player_snapshot_t));

    return player;
}

void player_destroy(player_t *player) {
    if (player) {
        free(player);
    }
}

entity_t * player_entity_spawn(const entity_manager_t *entityManager, player_t *player, GFC_Vector2D pos, const char *sprite) {
    entity_t *ent;
    Sprite *spriteImage;

    ent = entity_new_animated(entityManager);
    ent->position = pos;

    if (g_game.role == GAME_ROLE_CLIENT) {
        if (!sprite) {
            return NULL;
        }

        spriteImage = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);

        animation_createAnimation("idle", 0, 15, 0.1f, ANIMATION_TYPE_LOOP);
        ent->model = animation_sprite_createSprite(spriteImage, "idle");
        animation_state_setAnimationByName(((AnimatedSprite *)ent->model)->state, (AnimatedSprite *)ent->model, "idle");

        if (!ent->model) {
            log_error("failed to load entity sprite: '%s'", sprite);
            entity_free(entityManager, ent);
            return NULL;
        }

        camera_set_target(&g_camera, ent);
    }

    world_add_entity(g_game.world, ent);
    ent->data = player;

    ent->think = player_think;
    ent->update = player_update;
    ent->draw = player_draw;

    return ent;
}

GFC_Vector2D player_input_apply(player_t *player, const GFC_Vector2D position, const player_input_actions_t *actions, const float deltaTime, const uint8_t sync) {
    GFC_Vector2D newPos;
    if (!player || !actions) {
        return position;
    }

    GFC_Vector2D direction = gfc_vector2d(0.0f, 0.0f);
    direction.y = (actions->up ? -1.0f : 0.0f) + (actions->down ? 1.0f : 0.0f);
    direction.x = (actions->right ? 1.0f : 0.0f) + (actions->left ? -1.0f : 0.0f);

    if (direction.x == 0.0f && direction.y == 0.0f) {
        return position; // No movement input, skip processing
    }

    newPos = player_move(position, direction, PLAYER_SPEED, deltaTime);

    if (sync) {
        player_snapshot_t snapshot = {
            .position = newPos,
            .cmd = {
                .tickNumber = g_game.tickNumber,
                .axisX = (int32_t)direction.x,
                .axisY = (int32_t)direction.y
            }
        };
        buf_spsc_ring_push(player->inputBuffer, &snapshot);

        c2s_player_input_snapshot_packet_t pkt;
        create_c2s_player_input_snapshot(&pkt, &snapshot.cmd);
        client_send_to_server(&g_client, &pkt, 0);
    }

    return newPos;
}

void player_input_process(player_t *player, const player_input_command_t *cmd, const float deltaTime) {
    GFC_Vector2D direction;
    if (!player || !cmd) {
        return;
    }

    direction = gfc_vector2d((float)cmd->axisX, (float)cmd->axisY);
    player->position = player_move(player->position, direction, PLAYER_SPEED, deltaTime);
    player->lastProcessedInputTick = cmd->tickNumber;
    player->processedInput = 1;
}

void player_input_process_server(player_t *player, uint64_t tick, float xPos, float yPos) {
    GFC_Vector2D diverge, predPosition = gfc_vector2d(xPos, yPos), inputDirection;
    player_snapshot_t snapshot, *peeked;
    uint32_t idx, tail;
    uint8_t found = 0;
    float divergenceDistance;
    if (!player) {
        return;
    }

    while (buf_spsc_ring_peek(player->inputBuffer, &snapshot)) {
        if (snapshot.cmd.tickNumber < tick) {
            buf_spsc_ring_pop(player->inputBuffer, &snapshot);
        } else if (snapshot.cmd.tickNumber == tick) {
            found = 1;
            break;
        } else {
            break;
        }
    }

    if (!found) {
        return;
    }

    buf_spsc_ring_pop(player->inputBuffer, &snapshot);

    gfc_vector2d_sub(diverge, predPosition, snapshot.position);
    divergenceDistance = gfc_vector2d_magnitude(diverge);

    if (divergenceDistance > MAX_DIVERSION) {
        if (divergenceDistance < MAX_TELEPORT_DISTANCE) {
            idx = buf_spsc_ring_head_index(player->inputBuffer);
            tail = buf_spsc_ring_tail_index(player->inputBuffer);
            while (idx != tail) {
                peeked = (player_snapshot_t *)buf_spsc_ring_get(player->inputBuffer, idx);
                inputDirection = gfc_vector2d((float)peeked->cmd.axisX, (float)peeked->cmd.axisY);
                predPosition = player_move(predPosition, inputDirection, PLAYER_SPEED, g_game.deltaTime);
                idx = buf_spsc_ring_next_index(player->inputBuffer, idx);
            }
        } else {
            while (buf_spsc_ring_pop(player->inputBuffer, &snapshot)) {}
        }

        // TODO: lerp to predPosition instead of snapping for smoother correction
        player->position = predPosition;
    }
}

GFC_Vector2D player_move(GFC_Vector2D position, GFC_Vector2D direction, const float speed, const float deltaTime) {
    GFC_Vector2D moveDelta;
    gfc_vector2d_normalize(&direction);
    gfc_vector2d_scale(moveDelta, direction, speed * deltaTime);
    gfc_vector2d_add(position, position, moveDelta);
    return position;
}

int player_inventory_transaction(player_t *player, const inventory_transaction_t *transaction) {
    if (!player || !transaction) {
        return 0;
    }

    if (!inventory_transaction_try(&player->inventory, transaction) && !__INF_RESOURCES) {
        return 0;
    }

    inventory_transaction_apply(&player->inventory, transaction);
    network_session_add_transaction(player->data, transaction);

    return 1;
}

int player_try_build_tower(player_t *player, const struct tower_def_s *towerDef, const GFC_Vector2D position) {
    inventory_transaction_t *transaction;
    entity_t *entity;
    tower_state_t *tower;
    s2c_tower_create_packet_t towerPkt;
    if (!player || !towerDef) {
        return 0;
    }

    transaction = tower_get_cost_transaction(towerDef, 0);
    log_info("transaction with %d items for building tower with definition index %u", transaction->numItems, towerDef->index);
    if (!player_inventory_transaction(player, transaction)) {
        log_info("Player ID %u cannot afford to build tower with definition index %u", player->id, towerDef->index);
        return 0;
    }

    entity = tower_create_by_def(g_server.entityManager, g_server.towerManager, towerDef, position);
    tower = (tower_state_t *) entity->data;
    if (entity) {
        create_s2c_tower_create(&towerPkt, position.x, position.y, towerDef->index, tower->id);
        server_broadcast_packet(&g_server, &towerPkt, NET_UDP_FLAG_RELIABLE);
        return 1;
    }

    return 0;
}

void player_think(const entity_manager_t *entityManager, entity_t *ent) {
    if (!ent || !ent->data) {
        return;
    }

    // Placeholder for future thinking logic (e.g., AI, state changes)
}

void player_update(const entity_manager_t *entityManager, entity_t *ent, float deltaTime) {
    GFC_Vector2D position, mousePosition;
    player_t *player;
    player_input_actions_t actions;
    if (!ent || !ent->data) {
        return;
    }

    player = (player_t *)ent->data;

    if (g_game.role == GAME_ROLE_CLIENT) {
        entity_update_animated(entityManager, ent, deltaTime);

        actions.up = gfc_input_command_down("up");
        actions.down = gfc_input_command_down("down");
        actions.left = gfc_input_command_down("left");
        actions.right = gfc_input_command_down("right");

        position = player->position;
        camera_get_mouse_world_position(&g_camera, &mousePosition);
        actions.rotation = atan2f(mousePosition.y - position.y, mousePosition.x - position.x) * 180.0f / M_PI;

        player->position = player_input_apply(player, player->position, &actions, deltaTime, 1);
    } else if (player->processedInput) {
        player->processedInput = 0;

        s2c_player_state_snapshot_packet_t pkt;
        create_s2c_player_state_snapshot(&pkt, player->lastProcessedInputTick, player->position.x, player->position.y);
        server_send_packet(&g_server, player, &pkt, 0);
    }
}

void player_draw(const entity_manager_t *entityManager, entity_t *ent) {
    if (!ent || !ent->data) {
        return;
    }

    ent->position = ((player_t *)ent->data)->position;
    entity_draw_animated(entityManager, ent);
}