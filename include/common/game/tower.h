#ifndef TOWER_H
#define TOWER_H

#include <stdint.h>

#include "entity.h"
#include "gfc_vector.h"
#include "inventory.h"
#include "item.h"
#include "client/gf2d_sprite.h"

#define TOWER_MAX_LEVEL 5

#define TOWER_WEAPON_FLAG_DIRECTIONAL 0x01 // Indicates the tower has a directional weapon
#define TOWER_WEAPON_FLAG_PIERCING    0x02 // Indicates the tower's weapon slows enemies
#define TOWER_WEAPON_FLAG_AREA_EFFECT 0x04 // Indicates the tower's weapon affects an area

#define TOWER_EVENT_SHOOT 0x01 // Event ID for when a tower shoots

struct def_manager_s;
struct entity_manager_s;
struct player_s;

typedef enum tower_type_e {
    TOWER_TYPE_DEFENSIVE = 0,
    TOWER_TYPE_GOLD_PRODUCTION = 1,
    TOWER_TYPE_GATHERING = 2,
    TOWER_TYPE_PASSIVE = 3,
    TOWER_TYPE_STASH = 4,
} tower_type_t;

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
    uint32_t index;
    char name[32];
    char description[256];
    tower_type_t type;
    float size;
    float maxHealth[TOWER_MAX_LEVEL];
    int numWeapons;
    const tower_weapon_def_t *weaponDefs;
    float productionRate[TOWER_MAX_LEVEL];
    int productionAmount[TOWER_MAX_LEVEL];
    item_t cost[TOWER_MAX_LEVEL][3];
    tower_model_def_t modelDef;
} tower_def_t;

typedef struct tower_state_s {
    uint32_t id;
    const tower_def_t *def;
    float health;
    int level;
    float attackCooldown;
    float productionCooldown;
    GFC_Vector2D worldPos;
    struct entity_s *entity;
    Sprite *baseSprite;
    Sprite *weaponSprite;
    GFC_Vector2D shootDirection;
    uint8_t canShoot;
} tower_state_t;

typedef struct tower_def_manager_s tower_def_manager_t;

typedef struct tower_manager_s tower_manager_t;

/**
 * @brief Initializes the tower system with a specified maximum number of towers.
 * Allocates memory for tower states and sets up free slot management.
 *
 * @param defManager A pointer to the tower definition manager to use for loading tower definitions.
 * @param maxTowers The maximum number of towers that can be active at once.
 * @return A pointer to the initialized tower manager, or NULL if initialization fails.
 */
tower_manager_t *tower_init(tower_def_manager_t *defManager, uint32_t maxTowers);

/**
 * @brief Cleans up the tower system, freeing allocated memory for tower states and definitions.
 * Should be called on program exit to prevent memory leaks.
 */
void tower_close(const tower_manager_t *towerManager);

/**
 * @brief Loads tower definitions from a specified file path. The file should contain
 * the necessary data to populate the tower_def_manager with valid tower definitions.
 *
 * @param defManager A pointer to the tower definition manager to populate with loaded definitions.
 * @param filePath The path to the file containing tower definitions.
 * @return A pointer to the loaded tower definition manager, or NULL if loading fails.
 */
tower_def_manager_t *tower_load_defs(const struct def_manager_s *defManager, char *filePath);

/**
 * @brief Retrieves a tower definition by its name. This function searches the tower_def_manager
 * for a definition matching the provided name and returns a pointer to it.
 *
 * @param towerManager A pointer to the tower manager to search within.
 * @param name The name of the tower definition to retrieve.
 * @return A pointer to the tower definition if found, or NULL if no matching definition exists.
 */
const tower_def_t *tower_def_get(const tower_manager_t *towerManager, const char *name);

/**
 * @brief Retrieves a tower definition by its index. This function returns a pointer to the tower definition
 * at the specified index in the tower_def_manager's array of definitions.
 *
 * @param towerManager A pointer to the tower manager to search within.
 * @param index The index of the tower definition to retrieve.
 * @return A pointer to the tower definition if the index is valid, or NULL if the index is out of bounds.
 */
const tower_def_t *tower_def_get_by_index(const tower_manager_t *towerManager, int index);

/**
 * @brief Creates a new tower state based on a tower definition name and a specified position.
 * This function looks up the tower definition by name and initializes a new tower state with
 * the properties defined in the tower definition.
 *
 * @param entityManager A pointer to the entity manager to use for creating the tower's entity.
 * @param towerManager A pointer to the tower manager to use for creating the tower state.
 * @param name The name of the tower definition to use for creating the tower state.
 * @param position The world position where the tower should be created.
 * @return A pointer to the newly created tower state, or NULL if there are no free slots or if the definition is not found.
 */
entity_t *tower_create_by_name(const struct entity_manager_s *entityManager, tower_manager_t *towerManager, const char* name, GFC_Vector2D position);

/**
 * @brief Creates a new tower state based on a provided tower definition and a specified position.
 * This function initializes a new tower state with the properties defined in the provided tower definition.
 *
 * @param entityManager A pointer to the entity manager to use for creating the tower's entity.
 * @param towerManager A pointer to the tower manager to use for creating the tower state.
 * @param def A pointer to the tower definition to use for creating the tower state.
 * @param position The world position where the tower should be created.
 * @return A pointer to the newly created tower state, or NULL if there are no free slots.
 */
entity_t *tower_create_by_def(const struct entity_manager_s *entityManager, tower_manager_t *towerManager, const tower_def_t *def, GFC_Vector2D position);

/**
 * @brief Places a tower in the world at a specified position with a unique ID. This function initializes the tower state,
 * creates an associated entity, and sets up the tower's properties based on the provided definition.
 *
 * @param entityManager A pointer to the entity manager to use for creating the tower's entity.
 * @param towerManager A pointer to the tower manager to use for placing the tower.
 * @param def A pointer to the tower definition to use for placing the tower.
 * @param position The world position where the tower should be placed.
 * @param id The unique ID to assign to the tower.
 * @return A pointer to the newly placed tower state, or NULL if there are no free slots or if entity creation fails.
 */
entity_t *tower_place(const struct entity_manager_s *entityManager, tower_manager_t *towerManager, const tower_def_t *def, GFC_Vector2D position, uint32_t id);

void tower_request_upgrade(const struct entity_manager_s *entityManager, tower_manager_t *towerManager, entity_t *entity);
void tower_try_upgrade(const struct entity_manager_s *entityManager, tower_manager_t *towerManager, struct player_s *player, entity_t *entity);
void tower_upgrade(const struct entity_manager_s *entityManager, tower_manager_t *towerManager, entity_t *entity, int newLevel);
/**
 * @brief Retrieves a tower state by its unique ID. This function looks up the tower manager's mapping of tower IDs to their corresponding states
 * and returns a pointer to the tower state if found.
 *
 * @param towerManager A pointer to the tower manager to use for retrieving the tower state.
 * @param id The unique ID of the tower to retrieve.
 * @return A pointer to the tower state if found, or NULL if no tower with the specified ID exists.
 */
entity_t *tower_get_by_id(tower_manager_t *towerManager, uint32_t id);

/**
 * @brief Destroys a tower state, freeing any associated resources and marking its slot as free.
 * This function should be called when a tower is removed from the game to ensure proper cleanup.
 *
 * @param towerManager A pointer to the tower manager that manages the tower state.
 * @param entity A pointer to the tower state to destroy.
 */
void tower_destroy(tower_manager_t *towerManager, entity_t *entity);

/**
 * @brief Converts a string representation of a tower type to its corresponding enum value. This function compares the input string
 * against known tower type names and sets the output tower type accordingly.
 *
 * @param str The input string representing the tower type (e.g., "defensive", "production", "passive", "stash").
 * @param outType A pointer to a tower_type_t variable where the resulting enum value will be stored if the conversion is successful.
 */
void tower_type_from_string(const char *str, tower_type_t *outType);

/**
 * @brief Attempts to shoot with the tower's weapon. This function checks if the tower can shoot based on its cooldown
 * and updates the cooldown timer accordingly. If the tower can shoot, it returns a non-zero value; otherwise, it returns zero.
 *
 * @param entity A pointer to the tower state that is attempting to shoot.
 * @param deltaTime The time elapsed since the last update, used to update the cooldown timer.
 * @return A non-zero value if the tower can shoot, or zero if it is still cooling down.
 */
int tower_try_shoot(entity_t *entity, float deltaTime);

/**
 * @brief Executes the shooting action for a specific weapon index of the tower. This function spawns projectiles based on the weapon definition
 * and applies the appropriate properties such as damage, range, and direction.
 *
 * @param entityManager A pointer to the entity manager to use for spawning projectiles.
 * @param entity A pointer to the tower state that is shooting.
 * @param weaponIndex The index of the weapon to use for shooting, based on the tower's weapon definitions.
 */
void tower_shoot(const struct entity_manager_s *entityManager, entity_t *entity, int weaponIndex);

/**
 * @brief Executes the shooting action for all weapons of the tower. This function iterates through all the tower's weapons and calls the shooting function for each one.
 *
 * @param entityManager A pointer to the entity manager to use for spawning projectiles.
 * @param entity A pointer to the tower state that is shooting.
 */
void tower_shoot_all(const struct entity_manager_s *entityManager, entity_t *entity);

void tower_entity_draw_full(float size, GFC_Vector2D pos, Sprite *baseSprite, Sprite *weaponSprite, float rotation);

inventory_transaction_t *tower_get_cost_transaction(const tower_def_t *def, int level);

uint32_t tower_collides_with(entity_t *ent, entity_t *other);

GFC_Vector2D tower_snap_to_grid(const tower_def_t *towerDef, GFC_Vector2D position);

#endif /* TOWER_H */