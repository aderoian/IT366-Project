#ifndef COMMON_PLAYER_H
#define COMMON_PLAYER_H

#include "entity.h"
#include "gfc_vector.h"
#include "inventory.h"
#include "common/buffer/ring.h"
#include "common/network/packet/definitions.h"

#define INPUT_BUFFER_CAPACITY 256

#define PLAYER_SPEED 200.0f

struct tower_def_s;

typedef struct player_s {
    uint32_t id;
    char name[32];
    void *data;

    GFC_Vector2D position;
    inventory_t inventory;

    buf_spsc_ring_t *inputBuffer;
    uint64_t lastProcessedInputTick;
    uint8_t processedInput;
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

entity_t *player_entity_spawn(const struct entity_manager_s *entityManager, player_t *player, GFC_Vector2D pos, const char * sprite);

/**
 * @brief Applies player input actions to the player's state and optionally syncs with the server.
 * @environment CLIENT
 *
 * @param player The player to apply input for.
 * @param position The current position of the player, used for calculating movement and rotation.
 * @param actions The input actions to apply.
 * @param deltaTime The time elapsed since the last update, used for movement calculations.
 * @param sync Whether to sync the input command with the server (only true for local player).
 * @return The new position of the player after applying the input actions.
 */
GFC_Vector2D player_input_apply(player_t *player, GFC_Vector2D position, const player_input_actions_t *actions, float deltaTime, uint8_t sync);

/**
 * @brief Processes a player input command, applying it to the player's state.
 * @environment SERVER
 *
 * @param player The player to apply the input command for.
 * @param cmd The input command received from the client.
 * @param deltaTime The time elapsed since the last update, used for movement calculations.
 */
void player_input_process(player_t *player, const player_input_command_t *cmd, float deltaTime);

void player_input_process_server(player_t *player, uint64_t tick, float xPos, float yPos);

GFC_Vector2D player_move(GFC_Vector2D position, GFC_Vector2D direction, float speed, float deltaTime);

int player_inventory_transaction(player_t *player, const inventory_transaction_t *transaction);

int player_try_build_tower(player_t *player, const struct tower_def_s *towerDef, GFC_Vector2D position);

#endif /* COMMON_PLAYER_H */