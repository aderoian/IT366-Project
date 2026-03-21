#ifndef COLLISION_H
#define COLLISION_H
#include "entity.h"
#include "world/world.h"

int collision_check(const entity_t *a, const entity_t *b);

int collision_can_move(const world_t *world, const entity_t *ent, GFC_Vector2D newPosition);

#endif /* COLLISION_H */
