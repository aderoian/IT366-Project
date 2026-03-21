#ifndef COLLISION_H
#define COLLISION_H
#include "entity.h"
#include "world/world.h"

int collision_check(const entity_t *a, const entity_t *b);

int collision_check_world(const world_t *world, const entity_t *ent, GFC_Vector2D newPosition);

int collision_check_world_bounding(const world_t *world, GFC_Rect boundingBox);

#endif /* COLLISION_H */
