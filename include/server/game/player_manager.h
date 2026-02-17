#ifndef SERVER_PLAYER_MANAGER_H
#define SERVER_PLAYER_MANAGER_H

#include "common/game/player.h"

typedef struct player_manager_s player_manager_t;

player_manager_t *player_manager_create(size_t initialCapacity);
void player_manager_destroy(player_manager_t *manager);
player_t *player_manager_add(player_manager_t *manager, uint32_t id, const char *name);
void player_manager_remove(player_manager_t *manager, uint32_t id);
player_t *player_manager_get(player_manager_t *manager, uint32_t id);
const player_t **player_manager_get_all(player_manager_t *manager, size_t *outCount);

#endif /* SERVER_PLAYER_MANAGER_H */