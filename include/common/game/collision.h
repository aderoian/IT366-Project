#ifndef COLLISION_H
#define COLLISION_H
#include "entity.h"
#include "world/world.h"

#define COLLISION_NONE 0
#define COLLISION_SOLID 1
#define COLLISION_EVENT 2

int collision_check(const entity_t *a, const entity_t *b);

int collision_check_world(const world_t *world, const entity_t *ent, GFC_Vector2D newPosition);

int collision_check_world_bounding(const world_t *world, GFC_Rect boundingBox);\

int collision_raycast_world(const world_t *world, const entity_t *ent, GFC_Vector2D start, GFC_Vector2D end, GFC_List *hits);

GFC_List *collision_get_entities_in_range(const world_t *world, GFC_Vector2D position, float range, uint32_t layerMask);

#endif /* COLLISION_H */
