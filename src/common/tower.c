#include "common/tower.h"

#include "simple_json.h"
#include "common/def.h"
#include "common/logger.h"

tower_def_manager_t g_towerDefManager;
tower_manager_t g_towerManager;

void tower_init(const uint32_t maxTowers) {
    uint32_t i;
    g_towerManager.towers = gfc_allocate_array(sizeof(tower_t), maxTowers);
    g_towerManager.towerIDs = gfc_allocate_array(sizeof(uint32_t), maxTowers);
    g_towerManager.freeSlots = gfc_allocate_array(sizeof(uint32_t), maxTowers);
    g_towerManager.numTowers = 0;
    g_towerManager.numFreeSlots = maxTowers;
    g_towerManager.maxTowers = maxTowers;

    for (i = 0; i < maxTowers; i++) {
        g_towerManager.freeSlots[i] = maxTowers - 1 - i; // Fill free slots stack
        g_towerManager.towerIDs[i] = UINT32_MAX; // Mark all IDs as invalid
    }
}

void tower_close(void) {
    if (g_towerManager.towers) free(g_towerManager.towers);
    if (g_towerManager.towerIDs) free(g_towerManager.towerIDs);
    if (g_towerManager.freeSlots) free(g_towerManager.freeSlots);

    // Free tower definitions
    if (g_towerDefManager.towerDefs) free(g_towerDefManager.towerDefs);
}

const tower_def_t *tower_def_get(const char *name) {
    uint32_t i;
    for (i = 0; i < g_towerDefManager.numTowerDefs; i++) {
        if (strcmp(g_towerDefManager.towerDefs[i].name, name) == 0) {
            return &g_towerDefManager.towerDefs[i];
        }
    }
    return NULL; // Not found
}

tower_t *tower_create_by_name(const char* name, const GFC_Vector2D position) {
    const tower_def_t *def = tower_def_get(name);
    if (!def) {
        log_error("Tower definition not found for name: %s", name);
        return NULL;
    }
    return tower_create_by_def(def, position);
}

tower_t *tower_create_by_def(const tower_def_t *def, const GFC_Vector2D position) {
    tower_t * tower;
    if (g_towerManager.numFreeSlots == 0) {
        log_error("No free tower slots available");
        return NULL;
    }

    uint32_t slotIndex = g_towerManager.freeSlots[--g_towerManager.numFreeSlots];
    tower = &g_towerManager.towers[slotIndex];

    tower->id = slotIndex;
    tower->tilePos = position; // TODO: Proper tile position calculation based on world position and tile size
    tower->worldPos = position;
    tower->health = def->maxHealth[0];
    tower->towerDef = def;

    // TODO: create entity based on tower definition's modelDef and assign to tower->entity
    return tower;
}

void tower_destroy(tower_t *tower) {
    if (!tower) return;

    g_towerManager.freeSlots[g_towerManager.numFreeSlots++] = tower->id;
    tower->id = UINT32_MAX;

    if (tower->entity) {
        entity_free(tower->entity);
        tower->entity = NULL;
    }
}

void tower_load_defs(const char *filePath) {
    def_data_t *data = def_load(filePath), *towerJson, *towersArray;
    def_data_t *valueArray, *weaponJson, *weaponDefJson, *modelDefJson;
    tower_def_t *def;
    tower_weapon_def_t *weaponDef;
    const char * str;
    int i, j, k, flag;
    if (!data) {
        log_error("Failed to load tower definitions from file: %s", filePath);
        return;
    }

    towersArray = def_data_get_array(data, "towers");

    def_data_array_get_count(towersArray, &g_towerDefManager.numTowerDefs);
    g_towerDefManager.towerDefs = gfc_allocate_array(sizeof(tower_def_t), g_towerDefManager.numTowerDefs);
    for (i = 0; i < g_towerDefManager.numTowerDefs; i++) {
        towerJson = def_data_array_get_nth(towersArray, i);
        def = &g_towerDefManager.towerDefs[i];

        str = def_data_get_string(towerJson, "name");
        if (!str || strlen(str) >= sizeof(def->name)) {
            log_error("Invalid or missing name for tower definition at index %u", i);
            continue;
        }
        strncpy(def->name, str, sizeof(def->name) - 1);
        def->name[sizeof(def->name) - 1] = '\0';

        str = def_data_get_string(towerJson, "description");
        if (!str || strlen(str) >= sizeof(def->description)) {
            log_error("Invalid or missing description for tower definition at index %u", i);
            continue;
        }
        strncpy(def->description, str, sizeof(def->description) - 1);
        def->description[sizeof(def->description) - 1] = '\0';

        valueArray = def_data_get_array(towerJson, "maxHealth");
        for (j = 0; j < TOWER_MAX_LEVEL; j++) {
            sj_get_int32_value(def_data_array_get_nth(valueArray, j), &def->maxHealth[j]);
        }

        weaponJson = def_data_get_array(towerJson, "weapons");
        def_data_array_get_count(weaponJson, &def->numWeapons);
        def->weaponDefs = gfc_allocate_array(sizeof(tower_weapon_def_t), def->numWeapons);
        for (j = 0; j < def->numWeapons; ++j) {
            weaponDefJson = def_data_array_get_nth(weaponJson, j);
            weaponDef = &def->weaponDefs[j];

            valueArray = def_data_get_array(weaponDefJson, "damage");
            for (k = 0; k < TOWER_MAX_LEVEL; k++) {
                sj_get_float_value(def_data_array_get_nth(valueArray, k), &weaponDef->damage[k]);
            }

            valueArray = def_data_get_array(weaponDefJson, "range");
            for (k = 0; k < TOWER_MAX_LEVEL; k++) {
                sj_get_float_value(def_data_array_get_nth(valueArray, k), &weaponDef->range[k]);
            }

            valueArray = def_data_get_array(weaponDefJson, "fireRate");
            for (k = 0; k < TOWER_MAX_LEVEL; k++) {
                sj_get_float_value(def_data_array_get_nth(valueArray, k), &weaponDef->fireRate[k]);
            }

            valueArray = def_data_get_array(weaponDefJson, "bulletSpeed");
            for (k = 0; k < TOWER_MAX_LEVEL; k++) {
                sj_get_float_value(def_data_array_get_nth(valueArray, k), &weaponDef->bulletSpeed[k]);
            }

            weaponDef->flags = 0;
            def_data_get_int(weaponDefJson, "directional", &flag);
            if (flag) weaponDef->flags |= TOWER_WEAPON_FLAG_DIRECTIONAL;
            def_data_get_int(weaponDefJson, "piercing", &flag);
            if (flag) weaponDef->flags |= TOWER_WEAPON_FLAG_PIERCING;
            def_data_get_int(weaponDefJson, "areaEffect", &flag);
            if (flag) weaponDef->flags |= TOWER_WEAPON_FLAG_AREA_EFFECT;
            if (weaponDef->flags & TOWER_WEAPON_FLAG_DIRECTIONAL) {
                valueArray = def_data_get_array(weaponDefJson, "direction");
                sj_get_float_value(def_data_array_get_nth(valueArray, 0), &weaponDef->direction.x);
                sj_get_float_value(def_data_array_get_nth(valueArray, 1), &weaponDef->direction.y);
            }
        }

        modelDefJson = def_data_get_obj(towerJson, "model");

        str = def_data_get_string(modelDefJson, "base");
        if (!str || strlen(str) >= sizeof(def->spritePath)) {
            log_error("Invalid or missing spritePath for tower definition at index %u", i);
            continue;
        }

        strncpy(def->spritePath, str, sizeof(def->spritePath) - 1);
        def->spritePath[sizeof(def->spritePath) - 1] = '\0';

        str = def_data_get_string(modelDefJson, "head");
        if (!str || strlen(str) >= sizeof(def->modelDef.baseSpritePath)) {
            log_error("Invalid or missing modelDef.baseSpritePath for tower definition at index %u", i);
            continue;
        }

        strncpy(def->modelDef.baseSpritePath, str, sizeof(def->modelDef.baseSpritePath) - 1);
        def->modelDef.baseSpritePath[sizeof(def->modelDef.baseSpritePath) - 1] = '\0';
    }
}