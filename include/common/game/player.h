#ifndef COMMON_PLAYER_H
#define COMMON_PLAYER_H

#include "common/types.h"
typedef struct player_s {
    uint32_t id;
    char name[32];
    float position[3];
} player_t;

player_t *player_create(uint32_t id, const char *name);
void player_destroy(player_t *player);

#endif /* COMMON_PLAYER_H */