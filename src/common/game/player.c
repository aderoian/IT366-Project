#include "../../../include/common/game/player.h"

#include "gfc_types.h"

player_t *player_create(uint32_t id, const char *name) {
    player_t *player = (player_t *)gfc_allocate_array(sizeof(player_t), 1);
    if (!player) {
        return NULL;
    }

    player->id = id;
    memcpy(player->name, name, sizeof(name) + 1);
    player->position[0] = 0.0f;
    player->position[1] = 0.0f;
    player->position[2] = 0.0f;
    return player;
}

void player_destroy(player_t *player) {
    if (player) {
        free(player);
    }
}