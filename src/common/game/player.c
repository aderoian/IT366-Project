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

#include "server/server.h"

#define MAX_DIVERSION 1.5f
#define MAX_TELEPORT_DISTANCE 5.0f

typedef struct player_snapshot_s {
    GFC_Vector2D position;
    player_input_command_t cmd;
} player_snapshot_t;

void player_think(Entity *ent);
void player_update(Entity *ent, float deltaTime);
void player_draw(Entity *ent);

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

Entity * player_entity_spawn(player_t *player, GFC_Vector2D pos, const char *sprite) {
    Entity *ent;
    Sprite *spriteImage;

    ent = entity_new_animated();
    ent->position = pos;

    if (IS_CLIENT()) {
        if (!sprite) {
            return NULL;
        }

        spriteImage = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);

        animation_createAnimation("idle", 0, 15, 0.1f, ANIMATION_TYPE_LOOP);
        ent->model = animation_sprite_createSprite(spriteImage, "idle");
        animation_state_setAnimationByName(((AnimatedSprite *)ent->model)->state, (AnimatedSprite *)ent->model, "idle");

        if (!ent->model) {
            log_error("failed to load entity sprite: '%s'", sprite);
            entity_free(ent);
            return NULL;
        }

        camera_set_target(&g_camera, ent);
    }

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
        client_send_to_server(&g_client, PACKET_C2S_PLAYER_INPUT_SNAPSHOT, &pkt, 0);
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

void player_think(Entity *ent) {
    if (!ent || !ent->data) {
        return;
    }

    // Placeholder for future thinking logic (e.g., AI, state changes)
}

void player_update(Entity *ent, float deltaTime) {
    GFC_Vector2D position, mousePosition;
    player_t *player;
    player_input_actions_t actions;
    if (!ent || !ent->data) {
        return;
    }

    player = (player_t *)ent->data;

    if (IS_CLIENT()) {
        entity_update_animated(ent, deltaTime);

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
        server_send_packet(&g_server, player, PACKET_S2C_PLAYER_STATE_SNAPSHOT, &pkt, 0);
    }
}

void player_draw(Entity *ent) {
    if (!ent || !ent->data) {
        return;
    }

    ent->position = ((player_t *)ent->data)->position;
    entity_draw_animated(ent);
}