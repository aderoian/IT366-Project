#include "common/game/world/world.h"

#include "common/game/entity.h"
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

int world_add_entity(world_t *world, entity_t *ent) {
    if (!world || !ent) {
        return 0;
    }

    int chunkX = (int)(ent->position.x) >> CHUNK_POS_SHIFT;
    int chunkY = (int)(ent->position.y) >> CHUNK_POS_SHIFT;

    if (chunkX >= 0 && chunkX < world->size.x && chunkY >= 0 && chunkY < world->size.y) {
        chunk_add_entity(&world->chunks[chunkX * world->size.y + chunkY], ent);
        return 1;
    }

    return 0; // Entity position is out of world bounds
}

int world_move_entity(world_t *world, entity_t *ent, const GFC_Vector2D newPos) {
    if (!world || !ent) {
        return 0;
    }

    int newChunkX = (int)(newPos.x) >> CHUNK_POS_SHIFT;
    int newChunkY = (int)(newPos.y) >> CHUNK_POS_SHIFT;
    int oldChunkX = (int)(ent->position.x) >> CHUNK_POS_SHIFT;
    int oldChunkY = (int)(ent->position.y) >> CHUNK_POS_SHIFT;

    if (newChunkX != oldChunkX || newChunkY != oldChunkY) {
        if (newChunkX >= 0 && newChunkX < world->size.x && newChunkY >= 0 && newChunkY < world->size.y) {
            chunk_add_entity(&world->chunks[newChunkX * world->size.y + newChunkY], ent);
        } else {
            return 0; // New position is out of world bounds
        }

        if (oldChunkX >= 0 && oldChunkX < world->size.x && oldChunkY >= 0 && oldChunkY < world->size.y) {
            chunk_remove_entity(&world->chunks[oldChunkX * world->size.y + oldChunkY], ent);
        }
    }

    return 1;
}

int world_remove_entity(world_t *world, entity_t *ent) {
    if (!world || !ent) {
        return 0;
    }

    int chunkX = (int)(ent->position.x) >> CHUNK_POS_SHIFT;
    int chunkY = (int)(ent->position.y) >> CHUNK_POS_SHIFT;

    if (chunkX >= 0 && chunkX < world->size.x && chunkY >= 0 && chunkY < world->size.y) {
        chunk_remove_entity(&world->chunks[chunkX * world->size.y + chunkY], ent);
        return 1;
    }

    return 0; // Entity position is out of world bounds
}
