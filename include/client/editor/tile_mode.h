#ifndef TILE_MODE_H
#define TILE_MODE_H
#include <stdint.h>

#include "common/game/world/world.h"

#define TILE_MODE_SINGLE (1 << 0)
#define TILE_MODE_PAINT (1 << 1)

typedef struct tile_mode_s {
    uint32_t tileId;
    uint8_t mode;
    uint8_t active;

    world_t *world;
} tile_mode_t;

extern tile_mode_t g_tile_mode;

void tile_mode_set_mode(tile_mode_t* tm, uint8_t mode);

void tile_mode_set_tile(tile_mode_t *tm, uint32_t tileId);

void tile_mode_set_world(tile_mode_t *tm, world_t *world);

void tile_mode_activate(tile_mode_t *tm);

void tile_mode_deactivate(tile_mode_t *tm);

int tile_mode_process_mouse(tile_mode_t *tm, uint32_t mouseButton);

void tile_mode_update(tile_mode_t *tm, float deltaTime);

void tile_mode_draw(tile_mode_t *tm);

#endif /* TILE_MODE_H */