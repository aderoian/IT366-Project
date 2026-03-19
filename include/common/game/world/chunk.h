#ifndef CHUNK_H
#define CHUNK_H

#include "gfc_list.h"

struct world_s;

typedef struct chunk_s {
    int x;
    int y;
    GFC_List *entities;
} chunk_t;

chunk_t *chunk_create(int x, int y);

void chunk_destroy(chunk_t *chunk);

void chunk_initialize(chunk_t *chunk, int x, int y);

chunk_t *chunk_get(struct world_s *world, int x, int y);

int chunk_has_entity(const chunk_t *chunk, const void *entity);

void chunk_add_entity(chunk_t *chunk, void *entity);

void chunk_remove_entity(chunk_t *chunk, const void *entity);

#define world_to_chunk_pos(pos) (GFC_Vector2I){((int) pos.x) << CHUNK_POS_SHIFT, ((int) pos.y) << CHUNK_POS_SHIFT}

#endif /* CHUNK_H */