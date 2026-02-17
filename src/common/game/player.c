#include "../../../include/common/game/player.h"

#include "gfc_input.h"
#include "gfc_types.h"
#include "client/animation.h"
#include "client/camera.h"
#include "client/client.h"
#include "client/gf2d_sprite.h"
#include "common/logger.h"
#include "common/network/packet/definitions.h"
#include "common/network/packet/io.h"

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
    if (!sprite) return NULL;

    ent = entity_new_animated();
    ent->position = pos;
    spriteImage = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);

    animation_createAnimation("idle", 0, 15, 0.1f, ANIMATION_TYPE_LOOP);
    ent->model = animation_sprite_createSprite(spriteImage, "idle");
    animation_state_setAnimationByName(((AnimatedSprite *)ent->model)->state, (AnimatedSprite *)ent->model, "idle");

    if (!ent->model) {
        log_error("failed to load entity sprite: '%s'", sprite);
        entity_free(ent);
        return NULL;
    }

    ent->data = player;

    ent->update = player_update;
    ent->draw = player_draw;

    camera_set_target(&g_camera, ent);
}

void player_input_apply(player_t *player, const player_input_actions_t *actions, const float deltaTime, const uint8_t sync) {
    if (!player || !actions) {
        return;
    }

    GFC_Vector2D direction = gfc_vector2d(0.0f, 0.0f);
    direction.y = (actions->up ? -1.0f : 0.0f) + (actions->down ? 1.0f : 0.0f);
    direction.x = (actions->right ? 1.0f : 0.0f) + (actions->left ? -1.0f : 0.0f);

    if (direction.x == 0.0f && direction.y == 0.0f) {
        return; // No movement input, skip processing
    }

    player_move(player, direction, PLAYER_SPEED, deltaTime);

    if (sync) {
        player_input_command_t cmd;
        cmd.sequenceNumber = player->sequenceNumber++;
        cmd.axisX = (int32_t)direction.x; // Scale to int for network transmission
        cmd.axisY = (int32_t)direction.y;
        buf_spsc_ring_push(player->inputBuffer, &cmd);

        c2s_player_input_snapshot_packet_t pkt;
        create_c2s_player_input_snapshot(&pkt, &cmd);
        client_send_to_server(&g_client, PACKET_C2S_PLAYER_INPUT_SNAPSHOT, &pkt, 0);
    }
}

void player_input_process(player_t *player, player_input_command_t *cmd, const float deltaTime) {
    GFC_Vector2D direction;
    if (!player || !cmd) {
        return;
    }

    direction = gfc_vector2d((float)cmd->axisX, (float)cmd->axisY);
    player_move(player, direction, PLAYER_SPEED, deltaTime);

    s2c_player_state_snapshot_packet_t pkt;
    create_s2c_player_state_snapshot(&pkt, player->position.x, player->position.y);

}

void player_move(player_t *player, GFC_Vector2D direction, const float speed, const float deltaTime) {
    GFC_Vector2D moveDelta;
    if (!player) {
        return;
    }

    gfc_vector2d_normalize(&direction);
    gfc_vector2d_scale(moveDelta, direction, speed * deltaTime);
    gfc_vector2d_add(player->position, player->position, moveDelta);
}

void player_update(Entity *ent, float deltaTime) {
    GFC_Vector2D position, mousePosition;
    player_t *player;
    player_input_actions_t actions;
    if (!ent || !ent->data) {
        return;
    }

    player = (player_t *)ent->data;

    entity_update_animated(ent, deltaTime);

    actions.up = gfc_input_command_down("up");
    actions.down = gfc_input_command_down("down");
    actions.left = gfc_input_command_down("left");
    actions.right = gfc_input_command_down("right");

    position = player->position;
    camera_get_mouse_world_position(&g_camera, &mousePosition);
    actions.rotation = atan2f(mousePosition.y - position.y, mousePosition.x - position.x) * 180.0f / M_PI;

    player_input_apply(player, &actions, deltaTime, 1);
}

void player_draw(Entity *ent) {
    if (!ent || !ent->data) {
        return;
    }

    ent->position = ((player_t *)ent->data)->position;
    entity_draw_animated(ent);
}