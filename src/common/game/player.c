#include "gfc_input.h"
#include "gfc_types.h"

#include "common/logger.h"
#include "common/network/packet/definitions.h"
#include "common/network/packet/io.h"
#include "common/game/player.h"
#include "common/game/game.h"
#include "common/game/collision.h"
#include "common/game/tower.h"

#include "client/animation.h"
#include "client/camera.h"
#include "client/client.h"
#include "common/render/gf2d_draw.h"
#include "common/render/gf2d_sprite.h"

#include "server/server.h"
#include "server/network/network_session.h"

#define MAX_DIVERSION 1.5f
#define MAX_TELEPORT_DISTANCE 5.0f
#define PLAYER_ATTACK_COOLDOWN 0.5f
#define PLAYER_ATTACK_STRENGTH 60.0f

extern uint8_t __INF_RESOURCES, __DEBUG_LINES;

typedef struct player_snapshot_s {
    GFC_Vector2D position;
    player_input_command_t cmd;
} player_snapshot_t;

static uint8_t player_tile_allows_movement(const player_t *player, const GFC_Vector2D worldPos) {
    tile_t *tile;
    if (!g_game.world) {
        return 0;
    }

    tile = world_get_tile_at_position(g_game.world, worldPos, NULL);
    if (!tile) {
        return 0;
    }

    if (player && player->canFly) {
        return tile->properties.flyable;
    }

    return tile->properties.walkable;
}

static float player_tile_speed_multiplier(const GFC_Vector2D worldPos) {
    tile_t *tile;
    if (!g_game.world) {
        return 1.0f;
    }

    tile = world_get_tile_at_position(g_game.world, worldPos, NULL);
    if (!tile || !tile->properties.speed_modifier) {
        return 1.0f;
    }

    return fmaxf(0.0f, tile->properties.speedModifier);
}

static void player_apply_tile_effects(player_t *player, const GFC_Vector2D worldPos, const float deltaTime) {
    tile_t *tile;
    uint32_t tileId = 0;
    if (!player || !g_game.world) {
        return;
    }

    player->onHarmfulTile = 0;
    player->harmfulTileFeedbackTimer = fmaxf(0.0f, player->harmfulTileFeedbackTimer - deltaTime);

    tile = world_get_tile_at_position(g_game.world, worldPos, &tileId);
    if (!tile) {
        return;
    }

    if (tile->properties.harmful && tile->properties.damageAmount > 0.0f) {
        player->onHarmfulTile = 1;

        // Placeholder harmful-tile effect until a player health system exists.
        if (g_game.role == GAME_ROLE_SERVER && player->harmfulTileFeedbackTimer <= 0.0f) {
            log_info("Player %u touched harmful tile %u (%.2f damage placeholder)", player->id, tileId, tile->properties.damageAmount);
            player->harmfulTileFeedbackTimer = 1.0f;
        }
    }
}

void player_think(const entity_manager_t *entityManager, entity_t *ent);
void player_update(const entity_manager_t *entityManager, entity_t *ent, float deltaTime);
void player_draw(const entity_manager_t *entityManager, entity_t *ent);
uint32_t player_collides_with(entity_t *ent, entity_t *other);
uint32_t player_on_collide(entity_t *ent, entity_t *other, uint32_t collisionType);

player_t *player_create(uint32_t id, const char *name) {
    player_t *player = (player_t *)gfc_allocate_array(sizeof(player_t), 1);
    if (!player) {
        return NULL;
    }

    player->id = id;
    player->teamID = TEAM_NONE;
    memcpy(player->name, name, sizeof(name) + 1);
    player->position = gfc_vector2d(0.0f, 0.0f);

    player->inputBuffer = (buf_spsc_ring_t *)malloc(sizeof(buf_spsc_ring_t));
    buf_spsc_ring_init(player->inputBuffer, 64, sizeof(player_snapshot_t));
    player->canFly = 0;
    player->onHarmfulTile = 0;
    player->harmfulTileFeedbackTimer = 0.0f;

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

    ent = entity_new(entityManager, entity_next_id((entity_manager_t *)entityManager));
    ent->position = pos;

    if (g_game.role == GAME_ROLE_CLIENT) {
        if (!sprite) {
            return NULL;
        }

        ent->model = gf2d_sprite_load_image("images/player/player_base.svg");
        player->heldItem = gf2d_sprite_load_image("images/player/player_pickaxe.svg");

        if (!ent->model) {
            log_error("failed to load entity sprite: '%s'", sprite);
            entity_free(entityManager, ent);
            return NULL;
        }

        camera_set_target(&g_camera, ent);
    }

    ent->layers = ENT_LAYER_PLAYER;
    ent->boundingBox = gfc_rect(-36, -36, 72, 72);

    world_add_entity(g_game.world, ent);
    ent->data = player;
    player->entity = ent;

    ent->think = player_think;
    ent->update = player_update;
    ent->draw = player_draw;
    ent->collidesWith = player_collides_with;
    ent->onCollide = player_on_collide;

    return ent;
}

GFC_Vector2D player_input_apply(player_t *player, const GFC_Vector2D position, const player_input_actions_t *actions, const float deltaTime, const uint8_t sync) {
    GFC_Vector2D newPos;
    if (!player || !actions) {
        exit(1);
        return position;
    }

    int8_t dirX, dirY;
    dirX = (actions->right ? 1 : 0) - (actions->left ? 1 : 0);
    dirY = (actions->down ? 1 : 0) - (actions->up ? 1 : 0);
    GFC_Vector2D direction = gfc_vector2d(dirX, dirY);

    if (direction.x == 0.0f && direction.y == 0.0f && !actions->attack && player->entity->rotation == actions->rotation) {
        return position; // No input, skip processing
    }

    newPos = player_move(player, g_game.world, position, direction, PLAYER_SPEED, deltaTime);
    player_apply_tile_effects(player, newPos, deltaTime);

    if (sync) {
        player_snapshot_t snapshot = {
            .position = newPos,
            .cmd = {
                .tickNumber = g_game.tickNumber,
                .axisX = dirX, // Scale to fit in int8 range
                .axisY = dirY,
                .attack = actions->attack,
                .rotation = actions->rotation
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
    player->position = player_move(player, g_game.world, player->position, direction, PLAYER_SPEED, deltaTime);
    player_apply_tile_effects(player, player->position, deltaTime);

    player->entity->rotation = cmd->rotation;

    if (cmd->attack) {
        player_attack(player, g_game.world);
    }

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
                predPosition = player_move(player, g_game.world, predPosition, inputDirection, PLAYER_SPEED, g_game.deltaTime);
                idx = buf_spsc_ring_next_index(player->inputBuffer, idx);
            }
        } else {
            while (buf_spsc_ring_pop(player->inputBuffer, &snapshot)) {}
        }

        // TODO: lerp to predPosition instead of snapping for smoother correction
        player->position = predPosition;
    }
}

GFC_Vector2D player_move(player_t *player, world_t *world, GFC_Vector2D position, GFC_Vector2D direction, const float speed, const float deltaTime) {
    GFC_Vector2D moveDelta, newPosition;
    float effectiveSpeed;
    gfc_vector2d_normalize(&direction);
    effectiveSpeed = speed * player_tile_speed_multiplier(position);
    gfc_vector2d_scale(moveDelta, direction, effectiveSpeed * deltaTime);
    gfc_vector2d_add(newPosition, position, moveDelta);

    if (newPosition.x < 0 || newPosition.y < 0 || newPosition.x >= world->size.x * CHUNK_TILE_SIZE * TILE_SIZE || newPosition.y >= world->size.y * CHUNK_TILE_SIZE * TILE_SIZE) {
        return gfc_vector2d(fmaxf(0, fminf(newPosition.x, world->size.x * CHUNK_TILE_SIZE * TILE_SIZE - 1)), fmaxf(0, fminf(newPosition.y, world->size.y * CHUNK_TILE_SIZE * TILE_SIZE - 1)));
    }

    while (collision_check_world(world, player->entity, newPosition) || !player_tile_allows_movement(player, newPosition)) {
        gfc_vector2d_scale(moveDelta, moveDelta, 0.5f);
        gfc_vector2d_add(newPosition, position, moveDelta);

        if (gfc_vector2d_magnitude(moveDelta) < 0.01f) {
            return position; // Movement is too small, likely stuck, so give up
        }
    }

    return newPosition;
}

void player_attack(player_t *player, struct world_s *world) {
    GFC_Vector2D endPos;
    GFC_List *list;
    uint32_t i;
    item_t *item;
    if (!player || !world) {
        return;
    }

    if (player->attackCooldown > 0.0f) {
        return;
    }

    player->attackCooldown = PLAYER_ATTACK_COOLDOWN;
    endPos = gfc_vector2d_from_angle((player->entity->rotation - 180.0f) * M_PI / 180.0f);
    gfc_vector2d_scale(endPos, endPos, 50); // TODO: range based on tool
    gfc_vector2d_add(endPos, player->position, endPos);

    list = gfc_list_new();

    collision_raycast_world(world, player->entity, player->position, endPos, list);

    for (i = 0; i < gfc_list_count(list); i++) {
        entity_t *hitEnt = (entity_t *)gfc_list_nth(list, i);
        if (hitEnt->layers & ENT_LAYER_RESOURCE) {
            item = (item_t *)hitEnt->data;
            item = item_clone(item);
            item->quantity = 2;

            inventory_transaction_t *trans = inventory_transaction_create(1, 1);
            inventory_transaction_add_item(trans, item);
            player_inventory_transaction(player, trans);
        }
    }
}

int player_inventory_transaction(player_t *player, inventory_transaction_t *transaction) {
    if (!player || !transaction) {
        return 0;
    }

    if (!inventory_transaction_try(&player->inventory, transaction) && !__INF_RESOURCES) {
        inventory_transaction_destroy(transaction);
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
    if (!player || !towerDef) {
        return 0;
    }

    if (towerDef->type != TOWER_TYPE_STASH && g_game.state.phase == GAME_PHASE_EXPLORING) {
        return 0;
    }
    if (towerDef->type == TOWER_TYPE_STASH && g_game.state.phase != GAME_PHASE_EXPLORING) {
        return 0;
    }
    if (towerDef->type == TOWER_TYPE_STASH && g_game.state.mode == GAME_MODE_VERSUS) {
        if (player->teamID < TEAM_ONE || player->teamID > TEAM_TWO) {
            return 0;
        }
        if (g_game.state.teamStashAlive[player->teamID - TEAM_ONE]) {
            return 0;
        }
    }

    transaction = tower_get_cost_transaction(towerDef, 0);
    if (!player_inventory_transaction(player, transaction)) {
        log_info("Player ID %u cannot afford to build tower with definition index %u", player->id, towerDef->index);
        return 0;
    }

    entity = tower_create_by_def(g_game.entityManager, g_game.towerManager, towerDef, position);
    if (!entity) {
        return 0;
    }
    tower = (tower_state_t *) entity->data;
    tower->ownerPlayerID = player->id;
    tower->teamID = player->teamID;

    s2c_tower_snapshot_packet_t *towerPkt = gfc_allocate_array(sizeof(s2c_tower_snapshot_packet_t), 1);
    tower_snapshot_data_t towerData = {
        .createData = {
            .entityID = entity->id,
            .towerDefIndex = towerDef->index,
            .ownerPlayerID = tower->ownerPlayerID,
            .teamID = tower->teamID,
            .selectedEnemyDefIndex = tower->selectedEnemyDefIndex,
            .xPos = position.x,
            .yPos = position.y
        }
    };
    create_s2c_tower_snapshot(towerPkt, tower->id, TOWER_SNAPSHOT_CREATE, &towerData);
    server_broadcast_packet_batch(&g_server, towerPkt);

    if (towerDef->type == TOWER_TYPE_STASH) {
        if (g_game.state.mode == GAME_MODE_VERSUS) {
            uint8_t teamIndex;
            if (player->teamID < TEAM_ONE || player->teamID > TEAM_TWO) {
                return 0;
            }
            teamIndex = player->teamID - TEAM_ONE;
            if (g_game.state.teamStashAlive[teamIndex]) {
                return 0;
            }

            g_game.state.teamStashPositions[teamIndex] = position;
            g_game.state.teamStashTowerIDs[teamIndex] = tower->id;
            g_game.state.teamStashAlive[teamIndex] = 1;
            g_game.state.winnerTeamID = TEAM_NONE;
            if (g_game.state.teamStashAlive[0] && g_game.state.teamStashAlive[1]) {
                g_game.state.phase = GAME_PHASE_BUILDING;
                g_game.state.cycleTime = HALF_CYCLE_TIME;
            }
        } else {
            if (g_game.state.phase == GAME_PHASE_EXPLORING) {
                g_game.state.phase = GAME_PHASE_BUILDING;
                g_game.state.stashPosition = position;
            } else {
                // Stash towers start the game & players can only have one
                return 0;
            }
        }

        s2c_game_state_snapshot_packet_t snapshot;
        create_s2c_game_state_snapshot(&snapshot, &g_game.state);
        server_broadcast_packet(&g_server, &snapshot, NET_UDP_FLAG_RELIABLE);
    }

    return 1;
}

void player_think(const entity_manager_t *entityManager, entity_t *ent) {
    if (!ent || !ent->data) {
        return;
    }

    player_t *player = (player_t *)ent->data;
    player->attackCooldown -= g_game.deltaTime;
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
        actions.up = gfc_input_command_down("up");
        actions.down = gfc_input_command_down("down");
        actions.left = gfc_input_command_down("left");
        actions.right = gfc_input_command_down("right");
        actions.attack = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT);

        position = player->position;
        camera_get_mouse_world_position(&g_camera, &mousePosition);
        actions.rotation = atan2f(mousePosition.y - position.y, mousePosition.x - position.x) * 180.0f / M_PI + 90.0f;

        player->position = player_input_apply(player, player->position, &actions, deltaTime, 1);
        ent->rotation = actions.rotation;

        if (actions.attack && player->attackCooldown <= 0.0f) {
            player->attackCooldown = PLAYER_ATTACK_COOLDOWN;
        }
    } else if (player->processedInput) {
        player->processedInput = 0;

        s2c_player_state_snapshot_packet_t pkt;
        create_s2c_player_state_snapshot(&pkt, player->lastProcessedInputTick, player->position.x, player->position.y);
        server_send_packet(&g_server, player, &pkt, 0);
    }
}

void player_draw(const entity_manager_t *entityManager, entity_t *ent) {
    GFC_Vector2D position, centerPos, heldItemOffset, itemCenterPos;
    Sprite *playerSprite, *heldItemSprite;
    float handsRotation;
    if (!ent || !ent->data) {
        return;
    }

    player_t *player = (player_t *)ent->data;

    ent->position = player->position;
    gfc_vector2d_sub(position, ent->position, g_camera.position);

    playerSprite = ent->model;
    heldItemSprite = player->heldItem;
    centerPos = gfc_vector2d(playerSprite->frame_w * 0.5f, playerSprite->frame_h * 0.5f);
    heldItemOffset = gfc_vector2d(heldItemSprite->frame_w * 0.5f, heldItemSprite->frame_h);

    if (player->heldItem) {
        handsRotation = ent->rotation - PLAYER_ATTACK_STRENGTH * sinf(fmaxf(0.0f, player->attackCooldown / PLAYER_ATTACK_COOLDOWN * M_PI));
        gf2d_sprite_draw(heldItemSprite, position, NULL, &heldItemOffset, &handsRotation, NULL, NULL, 0);
    }

    gf2d_sprite_draw(ent->model, position,NULL, &centerPos, NULL, NULL, NULL, 0);

    if (__DEBUG_LINES) {
        GFC_Vector2D endPos;
        endPos = gfc_vector2d_from_angle((player->entity->rotation - 180.0f) * M_PI / 180.0f);
        gfc_vector2d_scale(endPos, endPos, 50); // TODO: range based on tool
        gfc_vector2d_add(endPos, player->position, endPos);
        gfc_vector2d_sub(endPos, endPos, g_camera.position);

        gf2d_draw_line(position, endPos, GFC_COLOR_DARKMAGENTA);
    }
}

uint32_t player_collides_with(entity_t *ent, entity_t *other) {
    if (!ent || !other || !ent->data) {
        return COLLISION_NONE;
    }

    if (other->layers & (ENT_LAYER_TOWER | ENT_LAYER_RESOURCE)) {
        return COLLISION_SOLID;
    }

    return COLLISION_NONE;
}

uint32_t player_on_collide(entity_t *ent, entity_t *other, uint32_t collisionType) {
    if (!ent || !other || !ent->data) {
        return collisionType;
    }

    if (collisionType == COLLISION_SOLID && (other->layers & ENT_LAYER_TOWER)) {
        tower_state_t *tower = (tower_state_t *)other->data;
        if (tower->def->type == TOWER_TYPE_PASSIVE && (strcmp(tower->def->name, "Door") == 0 || strcmp(tower->def->name, "Slow Trap") == 0)) {
            collisionType = COLLISION_NONE; // Allow passing through doors
        }
    }

    return collisionType;
}