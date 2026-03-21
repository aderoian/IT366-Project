#include "common/game/enemy.h"

#include "common/def.h"
#include "common/logger.h"

struct enemy_def_manager_s {
    enemy_def_t *enemyDefs;
    int numEnemyDefs;
};

enemy_def_manager_t * enemy_load_defs(const struct def_manager_s *defManager, char *filePath) {
    enemy_def_manager_t *enemyDefManager;
    def_data_t *data, *enmJson, *enmsArray, *valueArray, *modelJson;
    enemy_def_t *def;
    const char * str;
    float tmp;
    int i, j;
    enemyDefManager = gfc_allocate_array(sizeof(enemy_def_manager_t), 1);
    if (!enemyDefManager) {
        log_error("Failed to allocate memory for enemy definition manager");
        return NULL;
    }

    data = def_load(defManager, filePath);
    if (!data) {
        log_error("Failed to load enemy definitions from file: %s", filePath);
        free(enemyDefManager);
        return NULL;
    }

    enmsArray = def_data_get_array(data, "enemies");
    def_data_array_get_count(enmsArray, &enemyDefManager->numEnemyDefs);
    enemyDefManager->enemyDefs = gfc_allocate_array(sizeof(enemy_def_t), enemyDefManager->numEnemyDefs);
    for (i = 0; i < enemyDefManager->numEnemyDefs; i++) {
        enmJson = def_data_array_get_nth(enmsArray, i);
        def = &enemyDefManager->enemyDefs[i];

        str = def_data_get_string(enmJson, "name");
        if (!str || strlen(str) >= sizeof(def->name)) {
            log_error("Invalid or missing name for enemy definition at index %u", i);
            continue;
        }
        strncpy(def->name, str, sizeof(def->name) - 1);
        def->name[sizeof(def->name) - 1] = '\0';

        str = def_data_get_string(enmJson, "type");
        if (!str) {
            log_error("Missing type for enemy definition at index %u", i);
            continue;
        }
        def->type = enemy_type_from_string(str);

        valueArray = def_data_get_array(enmJson, "maxHealth");
        for (j = 0; j < ENEMY_MAX_LEVEL; j++) {
            sj_get_float_value(def_data_array_get_nth(valueArray, j), &def->maxHealth[j]);
        }
        valueArray = def_data_get_array(enmJson, "speed");
        for (j = 0; j < ENEMY_MAX_LEVEL; j++) {
            sj_get_float_value(def_data_array_get_nth(valueArray, j), &def->speed[j]);
        }
        valueArray = def_data_get_array(enmJson, "damage");
        for (j = 0; j < ENEMY_MAX_LEVEL; j++) {
            sj_get_float_value(def_data_array_get_nth(valueArray, j), &def->damage[j]);
        }
        valueArray = def_data_get_array(enmJson, "attackCooldown");
        for (j = 0; j < ENEMY_MAX_LEVEL; j++) {
            sj_get_float_value(def_data_array_get_nth(valueArray, j), &def->attackCooldown[j]);
        }
        valueArray = def_data_get_array(enmJson, "range");
        for (j = 0; j < ENEMY_MAX_LEVEL; j++) {
            sj_get_float_value(def_data_array_get_nth(valueArray, j), &def->range[j]);
        }
        valueArray = def_data_get_array(enmJson, "reward");
        for (j = 0; j < ENEMY_MAX_LEVEL; j++) {
            sj_get_int32_value(def_data_array_get_nth(valueArray, j), &def->reward[j]);
        }

        modelJson = def_data_get_obj(enmJson, "model");
        str = def_data_get_string(modelJson, "body");
        if (!str || strlen(str) >= sizeof(def->modelDef.bodySpritePath)) {
            log_error("Invalid or missing body spritePath for enemy definition at index %u", i);
            continue;
        }
        strncpy(def->modelDef.bodySpritePath, str, sizeof(def->modelDef.bodySpritePath) - 1);
        def->modelDef.bodySpritePath[sizeof(def->modelDef.bodySpritePath) - 1] = '\0';

        str = def_data_get_string(modelJson, "hands");
        if (str) {
            if (!str || strlen(str) >= sizeof(def->modelDef.handsSpritePath)) {
                log_error("Invalid or missing hands spritePath for enemy definition at index %u", i);
                continue;
            }
            strncpy(def->modelDef.handsSpritePath, str, sizeof(def->modelDef.handsSpritePath) - 1);
            def->modelDef.handsSpritePath[sizeof(def->modelDef.handsSpritePath) - 1] = '\0';
        }

        valueArray = def_data_get_array(modelJson, "boundingBox");
        if (valueArray) { // TODO: add double method to simple json
            sj_get_float_value(def_data_array_get_nth(valueArray, 0), &tmp);
            def->modelDef.boundingBox.x = tmp;
            sj_get_float_value(def_data_array_get_nth(valueArray, 1), &tmp);
            def->modelDef.boundingBox.y = tmp;
            sj_get_float_value(def_data_array_get_nth(valueArray, 2), &tmp);
            def->modelDef.boundingBox.w = tmp;
            sj_get_float_value(def_data_array_get_nth(valueArray, 3), &tmp);
            def->modelDef.boundingBox.h = tmp;
        }
    }
}

const enemy_def_t * enemy_def_get(const enemy_def_manager_t *enemyManager, const char *name) {
    int i;
    for (i = 0; i < enemyManager->numEnemyDefs; i++) {
        if (strcmp(enemyManager->enemyDefs[i].name, name) == 0) {
            return &enemyManager->enemyDefs[i];
        }
    }
    return NULL; // Not found
}

const enemy_def_t * enemy_def_get_by_index(const enemy_def_manager_t *enemyManager, int index) {
    if (index < 0 || index >= enemyManager->numEnemyDefs) {
        return NULL; // Invalid index
    }
    return &enemyManager->enemyDefs[index];
}

entity_t * enemy_create_by_def(const struct entity_manager_s *entityManager, const enemy_def_t *def, GFC_Vector2D pos) {
    entity_t *ent = enemy_spawn(entityManager, def, pos);
    if (!ent) {
        log_error("Failed to create enemy entity at position (%f, %f)", pos.x, pos.y);
        return NULL;
    }
    return ent;
}

entity_t * enemy_create_by_name(const struct entity_manager_s *entityManager, const enemy_def_manager_t *enemyManager,
    const char *name, GFC_Vector2D pos) {
    const enemy_def_t *def = enemy_def_get(enemyManager, name);
    if (!def) {
        log_error("Enemy definition not found for name: %s", name);
        return NULL;
    }
    return enemy_create_by_def(entityManager, def, pos);
}

entity_t * enemy_spawn(const struct entity_manager_s *entityManager, const enemy_def_t *def, GFC_Vector2D pos) {
}

enemy_type_t enemy_type_from_string(const char *str) {
    if (strcmp(str, "ground") == 0) {
        return ENEMY_TYPE_GROUND;
    }
    if (strcmp(str, "air") == 0) {
        return ENEMY_TYPE_AIR;
    }
    log_error("Invalid enemy type string: '%s'", str);
    return ENEMY_TYPE_GROUND; // Default to ground if invalid
}
