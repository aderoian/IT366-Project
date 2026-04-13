#ifndef WORLD_H
#define WORLD_H

#include "chunk.h"
#include "gfc_vector.h"
#include "common/game/item.h"
#include "common/game/world/tile.h"

#define TILE_SIZE 48

struct def_manager_s;
struct entity_s;
struct overlay_element_s;

typedef enum world_object_type_e {
    WORLD_OBJECT_NONE,
    WORLD_OBJECT_TOWER,
    WORLD_OBJECT_ENEMY,
    WORLD_OBJECT_RESOURCE
} world_object_type_t;

typedef struct selected_tower_s {
    struct entity_s *tower;
    int upgradeLevel;
    struct overlay_element_s *element;
} selected_tower_t;

typedef struct world_s {
    GFC_Vector2I size;
    uint8_t local;

    struct chunk_s *chunks;

    selected_tower_t *selected_tower;
} world_t;

world_t *world_create_empty(int width, int height);

world_t *world_create_from_file(const char* file);

void world_save(world_t *world, const char* file);

void world_destroy(world_t *world);

chunk_t *world_get_chunk(const world_t *world, int x, int y);

tile_t *world_get_tile_at_position(const world_t *world, GFC_Vector2D worldPos, uint32_t *outTileId);

void world_update(world_t *world, float deltaTime);

int world_on_click(world_t *world, uint32_t mouseButton, int x, int y);

void world_clear(world_t *world);

void world_draw(const world_t *world);

int world_add_entity(world_t *world, struct entity_s *ent);

int world_move_entity(world_t *world, struct entity_s *ent, GFC_Vector2D newPos);

int world_remove_entity(world_t *world, struct entity_s *ent);

world_object_type_t world_type_from_string(const char *typeStr);

struct entity_s *world_object_resource_spawn(world_t *world, GFC_Vector2D pos, item_t *resource, const char *image);

#define pos_to_chunk_coord(pos) ((int)(((int) (pos)) / (CHUNK_TILE_SIZE * TILE_SIZE)))

#endif /* WORLD_H */