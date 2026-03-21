#ifndef ENEMY_H
#define ENEMY_H

#include <stdint.h>

#include "entity.h"
#include "gfc_shape.h"

#define ENEMY_MAX_LEVEL 5

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

    GFC_List *targets;
    uint8_t dirty; // Flag to indicate if the enemy state has changed and needs to be synchronized with clients
} enemy_state_t;

typedef struct enemy_def_manager_s enemy_def_manager_t;

enemy_def_manager_t *enemy_load_defs(const struct def_manager_s *defManager, char *filePath);

const enemy_def_t *enemy_def_get(const enemy_def_manager_t *enemyManager, const char *name);

const enemy_def_t *enemy_def_get_by_index(const enemy_def_manager_t *enemyManager, int index);

entity_t *enemy_create_by_def(const struct entity_manager_s *entityManager, const enemy_def_t *def, GFC_Vector2D pos);

entity_t *enemy_create_by_name(const struct entity_manager_s *entityManager, const enemy_def_manager_t *enemyManager, const char* name, GFC_Vector2D pos);

entity_t *enemy_spawn(const struct entity_manager_s *entityManager, const enemy_def_t *def, GFC_Vector2D pos);

enemy_type_t enemy_type_from_string(const char *str);

#endif /* ENEMY_H */