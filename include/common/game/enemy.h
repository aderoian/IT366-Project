#ifndef ENEMY_H
#define ENEMY_H

#include <stdint.h>

#include "entity.h"
#include "gfc_shape.h"
#include "client/gf2d_sprite.h"

#define ENEMY_MAX_LEVEL 5

#define ENEMY_DIRTY_POSITION 0x0001
#define ENEMY_DIRTY_HEALTH 0x0002
#define ENEMY_DIRTY_ATTACK 0x0004

struct def_manager_s;
struct entity_manager_s;

typedef enum enemy_type_e {
    ENEMY_TYPE_GROUND = 0,
    ENEMY_TYPE_AIR = 1,
} enemy_type_t;

typedef struct enemy_model_def_s {
    char bodySpritePath[64];
    char handsSpritePath[64];
    GFC_Rect boundingBox;
} enemy_model_def_t;

typedef struct enemy_def_s {
    uint32_t index;
    char name[32];
    enemy_type_t type;
    float maxHealth[ENEMY_MAX_LEVEL];
    float speed[ENEMY_MAX_LEVEL];
    float damage[ENEMY_MAX_LEVEL];
    float attackCooldown[ENEMY_MAX_LEVEL];
    float range[ENEMY_MAX_LEVEL];
    int reward[ENEMY_MAX_LEVEL];
    enemy_model_def_t modelDef;
} enemy_def_t;

typedef struct enemy_state_s {
    uint32_t id;
    const enemy_def_t *def;
    float health;
    int level;
    GFC_Vector2D worldPos;
    struct entity_s *entity;

    Sprite *bodySprite;
    Sprite *handsSprite;

    float attackCooldownTimer;
    float attackTargetTimer;

    GFC_List *targets;
    uint32_t dirtyFlags; // Bitfield for tracking what needs to be updated on clients (e.g., position, health, targets)
} enemy_state_t;

typedef struct enemy_def_manager_s enemy_def_manager_t;

enemy_def_manager_t *enemy_load_defs(const struct def_manager_s *defManager, char *filePath);

const enemy_def_t *enemy_def_get(const enemy_def_manager_t *enemyManager, const char *name);

const enemy_def_t *enemy_def_get_by_index(const enemy_def_manager_t *enemyManager, int index);

entity_t *enemy_create_by_def(const struct entity_manager_s *entityManager, const enemy_def_t *def, GFC_Vector2D pos);

entity_t *enemy_create_by_name(const struct entity_manager_s *entityManager, const enemy_def_manager_t *enemyManager, const char* name, GFC_Vector2D pos);

entity_t *enemy_spawn(const struct entity_manager_s *entityManager, const enemy_def_t *def, GFC_Vector2D pos);

enemy_type_t enemy_type_from_string(const char *str);

void enemy_think(const entity_manager_t *entityManager, entity_t *ent);
void enemy_update(const entity_manager_t *entityManager, entity_t *ent, float deltaTime);
void enemy_draw(const entity_manager_t *entityManager, entity_t *ent);
void enemy_destroy(const entity_manager_t *entityManager, entity_t *ent);
uint32_t enemy_collides_with(entity_t *ent, entity_t *other);

#endif /* ENEMY_H */