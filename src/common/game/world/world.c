#include "common/game/world/world.h"
#include "common/game/world/chunk.h"

world_t *world_create(const int width, const int height, const uint8_t local) {
    int i, j;
    world_t *world = malloc(sizeof(world_t));
    if (!world) {
        return NULL;
    }

    world->chunks = gfc_allocate_array(sizeof(chunk_t), width * height);
    if (!world->chunks) {
        free(world);
        return NULL;
    }

    for (i = 0; i < width; i++) {
        for (j = 0; j < height; j++) {
            chunk_initialize(&world->chunks[i * height + j], i, j);
        }
    }

    world->size.x = width; world->size.y = height;
    world->local = local;

    return world;
}

void world_destroy(world_t *world) {
    int i, j;
    if (!world) {
        return;
    }

    for (i = 0; i < world->size.x; i++) {
        for (j = 0; j < world->size.y; j++) {
            chunk_destroy(&world->chunks[i * world->size.y + j]);
        }
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
