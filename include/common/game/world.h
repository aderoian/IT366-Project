#ifndef WORLD_H
#define WORLD_H
#include "gfc_vector.h"

#define TILE_SIZE 32

typedef struct world_s {
    GFC_Vector2I size;
    uint8_t local;
} world_t;

world_t *world_create(int width, int height, uint8_t local);

void world_destroy(world_t *world);

void world_update(world_t *world, float deltaTime);
void world_draw(const world_t *world);

#endif /* WORLD_H */