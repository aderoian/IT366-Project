#ifndef WORLD_TILE_H
#define WORLD_TILE_H

#include <stdint.h>

#include "../../render/gf2d_sprite.h"

typedef struct tile_properties_s {
    short walkable;
    short flyable;
    short buildable;
    short harmful;
    short modifySpeed;

    float damageAmount;
    float speedModifier;
} tile_properties_t;

typedef struct tile_s {
    uint32_t id;
    tile_properties_t properties;
    Sprite *sprite;
    uint32_t spriteFrame;
} tile_t;

typedef struct tile_manager_s tile_manager_t;

tile_manager_t *tile_manager_init(const char *file);

void tile_manager_free(tile_manager_t *tm);

tile_t *tile_manager_get(tile_manager_t *manager, uint32_t id);

int tile_test_flag(tile_manager_t *manager, uint32_t id, uint64_t flag);

void tile_draw_tile(tile_manager_t *manager, uint32_t id, int x, int y);

void tile_draw_tile_surface(tile_manager_t *manager, uint32_t id, int x, int y, SDL_Surface *surface);

#endif /* WORLD_TILE_H */