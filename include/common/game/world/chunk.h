#ifndef CHUNK_H
#define CHUNK_H

#include "gfc_list.h"

#define CHUNK_TILE_SIZE 16

struct world_s;

typedef struct chunk_s {
    int x;
    int y;
    uint32_t tiles[CHUNK_TILE_SIZE][CHUNK_TILE_SIZE];
    GFC_List *entities;
    SDL_Texture *texture;
} chunk_t;

chunk_t *chunk_create(int x, int y);

void chunk_destroy(chunk_t *chunk);

void chunk_initialize(chunk_t *chunk, int x, int y, void *data);

void chunk_serialize(const chunk_t *chunk, uint32_t *data);

chunk_t *chunk_get(struct world_s *world, int x, int y);

int chunk_has_entity(const chunk_t *chunk, const void *entity);

void chunk_add_entity(chunk_t *chunk, void *entity);

void chunk_remove_entity(chunk_t *chunk, const void *entity);

SDL_Texture *chunk_create_texture(const chunk_t *chunk, SDL_Renderer *renderer);

void chunk_draw(const chunk_t *chunk);

#endif /* CHUNK_H */