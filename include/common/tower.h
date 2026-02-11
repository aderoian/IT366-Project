#ifndef TOWER_H
#define TOWER_H

#include <stdint.h>

#include "gfc_vector.h"

#include "common/entity.h"

#define TOWER_MAX_LEVEL 5

#define TOWER_WEAPON_FLAG_DIRECTIONAL 0x01 // Indicates the tower has a directional weapon
#define TOWER_WEAPON_FLAG_PIERCING    0x02 // Indicates the tower's weapon slows enemies
#define TOWER_WEAPON_FLAG_AREA_EFFECT 0x04 // Indicates the tower's weapon affects an area

typedef struct tower_weapon_def_s {
    float damage[TOWER_MAX_LEVEL];
    float range[TOWER_MAX_LEVEL];
    float fireRate[TOWER_MAX_LEVEL];
    float bulletSpeed[TOWER_MAX_LEVEL];
    uint32_t flags;
    GFC_Vector2D direction;
} tower_weapon_def_t;

typedef struct tower_model_def_s {
    char baseSpritePath[64];
    char weaponSpritePath[64];
} tower_model_def_t;

typedef struct tower_def_s {
    char name[32];
    char description[256];
    char spritePath[64];
    int maxHealth[TOWER_MAX_LEVEL];
    int numWeapons;
    const tower_weapon_def_t *weaponDefs;
    tower_model_def_t modelDef;
} tower_def_t;

typedef struct tower_s {
    uint32_t id;
    GFC_Vector2D tilePos;
    GFC_Vector2D worldPos;
    int health;
    const tower_def_t *towerDef;
    Entity *entity;
} tower_t;

typedef struct tower_def_manager_s {
    tower_def_t *towerDefs;
    int numTowerDefs;
} tower_def_manager_t;

typedef struct tower_manager_s {
    tower_t *towers;
    uint32_t *towerIDs; // Maps tower ID to index in towers array
    uint32_t *freeSlots; // Stack of free tower slots
    uint32_t numTowers;
    uint32_t numFreeSlots;
    uint32_t maxTowers;
} tower_manager_t;

extern tower_def_manager_t g_towerDefManager;
extern tower_manager_t g_towerManager;

void tower_init(uint32_t maxTowers);
void tower_close(void);

void tower_load_defs(const char *filePath);

const tower_def_t *tower_def_get(const char *name);

tower_t *tower_create_by_name(const char* name, GFC_Vector2D position);
tower_t *tower_create_by_def(const tower_def_t *def, GFC_Vector2D position);
void tower_destroy(tower_t *tower);

#endif /* TOWER_H */