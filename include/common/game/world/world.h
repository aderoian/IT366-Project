#ifndef WORLD_H
#define WORLD_H

#include "chunk.h"
#include "gfc_vector.h"

#define TILE_SIZE 48
#define CHUNK_TILE_SIZE 16

struct entity_s;

typedef struct world_s {
    GFC_Vector2I size;
    uint8_t local;

    struct chunk_s *chunks;

    SDL_Texture *chunkTexture; // Client-side texture for rendering chunks
} world_t;

world_t *world_create(int width, int height, uint8_t local);

void world_create_chunk_texture(world_t *world);

void world_destroy(world_t *world);

GFC_Vector2D world_pos_tile_snap(const world_t *world, GFC_Vector2D worldPos);

chunk_t *world_get_chunk(const world_t *world, int x, int y);

void world_update(world_t *world, float deltaTime);

void world_draw(const world_t *world);

int world_add_entity(world_t *world, struct entity_s *ent);
int world_move_entity(world_t *world, struct entity_s *ent, GFC_Vector2D newPos);
int world_remove_entity(world_t *world, struct entity_s *ent);

#define pos_to_chunk_coord(pos) ((int)(((int) (pos)) / (CHUNK_TILE_SIZE * TILE_SIZE)))

#endif /* WORLD_H */