#ifndef PLAYER_ENTITY_H
#define PLAYER_ENTITY_H

#include "common/entity.h"

Entity * player_spawn(GFC_Vector2D pos, const char * sprite);
Entity * player_spawn_immobile(GFC_Vector2D pos, const char * sprite);

void player_think(Entity *ent);

#endif /* PLAYER_ENTITY_H */