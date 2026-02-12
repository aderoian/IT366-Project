#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "gfc_vector.h"

struct tower_state_s;
struct Entity_S;

typedef struct projectile_state_s {
    float speed;
    int damage;
    float range;
    GFC_Vector2D direction;
    GFC_Vector2D distanceTraveled;
    struct tower_state_s *sourceTower;
    struct Entity_S *entity;
} projectile_state_t;

int projectile_spawn(float speed, float damage, float range, GFC_Vector2D direction, const char *spriteModel, struct tower_state_s *sourceTower);

#endif /* PROJECTILE_H */