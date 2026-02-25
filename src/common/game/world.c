#include "common/game/world.h"

world_t *world_create(int width, int height, const uint8_t local) {
    world_t *world = malloc(sizeof(world_t));
    if (!world) {
        return NULL;
    }

    if (width % TILE_SIZE != 0 || height % TILE_SIZE != 0) {
        width = ((width + TILE_SIZE - 1) / TILE_SIZE) * TILE_SIZE;
        height = ((height + TILE_SIZE - 1) / TILE_SIZE) * TILE_SIZE;
    }

    world->size.x = width; world->size.y = height;
    world->local = local;

    return world;
}

void world_destroy(world_t *world) {
    if (world) {
        free(world);
    }
}

GFC_Vector2D world_pos_tile_snap(const world_t *world, const GFC_Vector2D worldPos) {
    GFC_Vector2D tilePos;
    if (!world) {
        return (GFC_Vector2D){0, 0};
    }

    tilePos.x = floorf(worldPos.x / TILE_SIZE) * TILE_SIZE;
    tilePos.y = floorf(worldPos.y / TILE_SIZE) * TILE_SIZE;
    return tilePos;
}

void world_update(world_t *world, float deltaTime) {
    if (!world) {
        return;
    }
}

void world_draw(const world_t *world) {
    if (!world || !world->local) {
        return;
    }
}
