#ifndef TOWER_H
#define TOWER_H

#include <stdint.h>

#include "gfc_vector.h"

#include "entity.h"

#define TOWER_MAX_LEVEL 5

#define TOWER_WEAPON_FLAG_DIRECTIONAL 0x01 // Indicates the tower has a directional weapon
#define TOWER_WEAPON_FLAG_PIERCING    0x02 // Indicates the tower's weapon slows enemies
#define TOWER_WEAPON_FLAG_AREA_EFFECT 0x04 // Indicates the tower's weapon affects an area

typedef struct tower_weapon_def_s {
    float damage[TOWER_MAX_LEVEL];
    float range[TOWER_MAX_LEVEL];
    float fireRate[TOWER_MAX_LEVEL];
    float bulletSpeed[TOWER_MAX_LEVEL];
    char projectileSprite[64];
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
    float maxHealth[TOWER_MAX_LEVEL];
    int numWeapons;
    const tower_weapon_def_t *weaponDefs;
    tower_model_def_t modelDef;
} tower_def_t;

typedef struct tower_def_manager_s {
    tower_def_t *towerDefs;
    int numTowerDefs;
} tower_def_manager_t;

typedef struct tower_state_s {
    uint32_t id;
    const tower_def_t *def;
    float health;
    int level;
    float cooldown;
    GFC_Vector2I tilePos;
    GFC_Vector2D worldPos;
    struct Entity_S *entity;
} tower_state_t;

typedef struct tower_manager_s {
    tower_state_t *towers;
    uint32_t *towerIDs; // Maps tower ID to index in towers array
    uint32_t *freeSlots; // Stack of free tower slots
    uint32_t numTowers;
    uint32_t numFreeSlots;
    uint32_t maxTowers;
} tower_manager_t;

/** * Global managers for tower definitions and tower states */
extern tower_def_manager_t g_towerDefManager;
extern tower_manager_t g_towerManager;

/**
 * @brief Initializes the tower system with a specified maximum number of towers.
 * Allocates memory for tower states and sets up free slot management.
 *
 * @param maxTowers The maximum number of towers that can be active at once.
 */
void tower_init(uint32_t maxTowers);

/**
 * @brief Cleans up the tower system, freeing allocated memory for tower states and definitions.
 * Should be called on program exit to prevent memory leaks.
 */
void tower_close(void);

/**
 * @brief Loads tower definitions from a specified file path. The file should contain
 * the necessary data to populate the tower_def_manager with valid tower definitions.
 *
 * @param filePath The path to the file containing tower definitions.
 */
void tower_load_defs(const char *filePath);

/**
 * @brief Retrieves a tower definition by its name. This function searches the tower_def_manager
 * for a definition matching the provided name and returns a pointer to it.
 *
 * @param name The name of the tower definition to retrieve.
 * @return A pointer to the tower definition if found, or NULL if no matching definition exists.
 */
const tower_def_t *tower_def_get(const char *name);

/**
 * @brief Creates a new tower state based on a tower definition name and a specified position.
 * This function looks up the tower definition by name and initializes a new tower state with
 * the properties defined in the tower definition.
 *
 * @param name The name of the tower definition to use for creating the tower state.
 * @param position The world position where the tower should be created.
 * @return A pointer to the newly created tower state, or NULL if the definition is not found or if there are no free slots.
 */
tower_state_t *tower_create_by_name(const char* name, GFC_Vector2D position);

/**
 * @brief Creates a new tower state based on a provided tower definition and a specified position.
 * This function initializes a new tower state with the properties defined in the provided tower definition.
 *
 * @param def A pointer to the tower definition to use for creating the tower state.
 * @param position The world position where the tower should be created.
 * @return A pointer to the newly created tower state, or NULL if there are no free slots.
 */
tower_state_t *tower_create_by_def(const tower_def_t *def, GFC_Vector2D position);

/**
 * @brief Destroys a tower state, freeing any associated resources and marking its slot as free.
 * This function should be called when a tower is removed from the game to ensure proper cleanup.
 *
 * @param tower A pointer to the tower state to be destroyed.
 */
void tower_destroy(tower_state_t *tower);

int tower_try_shoot(tower_state_t *tower, float deltaTime);
void tower_shoot(tower_state_t *tower, int weaponIndex);
void tower_shoot_all(tower_state_t *tower);
#endif /* TOWER_H */