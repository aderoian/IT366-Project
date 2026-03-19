#include "common/game/world/chunk.h"

#include "common/game/world/world.h"

chunk_t * chunk_create(const int x, const int y) {
    chunk_t *chunk = malloc(sizeof(chunk_t));
    if (!chunk) {
        return NULL;
    }

    chunk->x = x;
    chunk->y = y;
    chunk->entities = gfc_list_new();
    if (!chunk->entities) {
        free(chunk);
        return NULL;
    }

    return chunk;
}

void chunk_destroy(chunk_t *chunk) {
    if (!chunk) {
        return;
    }

    if (chunk->entities) {
        gfc_list_clear(chunk->entities);
        free(chunk->entities);
    }

    free(chunk);
}

void chunk_initialize(chunk_t *chunk, const int x, const int y) {
    if (!chunk) {
        return;
    }

    chunk->x = x;
    chunk->y = y;

    if (!chunk->entities) {
        chunk->entities = gfc_list_new();
    } else {
        gfc_list_clear(chunk->entities);
    }
}

chunk_t * chunk_get(world_t *world, const int x, const int y) {
    if (!world || x < 0 || y < 0 || x >= world->size.x || y >= world->size.y) {
        return NULL;
    }

    return &world->chunks[x * world->size.y + y];
}

int chunk_has_entity(const chunk_t *chunk, const void *entity) {
    if (!chunk || !entity) {
        return -1;
    }

    return gfc_list_get_item_index(chunk->entities, (void*)entity);
}

void chunk_add_entity(chunk_t *chunk, void *entity) {
    if (!chunk || !entity) {
        return;
    }

    gfc_list_append(chunk->entities, entity);
}

void chunk_remove_entity(chunk_t *chunk, const void *entity) {
    if (!chunk || !entity) {
        return;
    }

    gfc_list_delete_data(chunk->entities, (void*)entity);
}
