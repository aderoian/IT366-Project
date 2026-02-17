#ifndef COMMON_PLAYER_H
#define COMMON_PLAYER_H

#include "entity.h"
#include "gfc_vector.h"
#include "common/types.h"
#include "common/buffer/ring.h"

#define INPUT_BUFFER_CAPACITY 256

#define PLAYER_SPEED 200.0f

typedef struct player_s {
    uint32_t id;
    char name[32];
    void *data;

    GFC_Vector2D position;

    buf_spsc_ring_t *inputBuffer;
    uint64_t sequenceNumber;
} player_t;

typedef struct player_input_actions_s {
    uint8_t up : 1;
    uint8_t down : 1;
    uint8_t left : 1;
    uint8_t right : 1;
    uint8_t use : 1;
    float rotation;
} player_input_actions_t;

player_t *player_create(uint32_t id, const char *name);

void player_destroy(player_t *player);

Entity *player_entity_spawn(player_t *player, GFC_Vector2D pos, const char * sprite);

/**
 * @brief Applies player input actions to the player's state and optionally syncs with the server.
 * @environment CLIENT
 *
 * @param player The player to apply input for.
 * @param actions The input actions to apply.
 * @param deltaTime The time elapsed since the last update, used for movement calculations.
 * @param sync Whether to sync the input command with the server (only true for local player).
 */
void player_input_apply(player_t *player, const player_input_actions_t *actions, float deltaTime, uint8_t sync);

/**
 * @brief Processes a player input command, applying it to the player's state.
 * @environment SERVER
 *
 * @param player The player to apply the input command for.
 * @param cmd The input command received from the client.
 * @param deltaTime The time elapsed since the last update, used for movement calculations.
 */
void player_input_process(player_t *player, player_input_command_t *cmd, float deltaTime);

void player_move(player_t *player, GFC_Vector2D direction, float speed, float deltaTime);


#endif /* COMMON_PLAYER_H */