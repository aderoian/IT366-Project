#include "simple_json.h"
#include "common/def.h"
#include "common/logger.h"

#include "common/game/tower.h"

#include "client/camera.h"
#include "client/client.h"
#include "client/gf2d_sprite.h"
#include "common/game/projectile.h"
#include "common/game/world/world.h"
#include "common/network/packet/definitions.h"
#include "common/network/packet/io.h"
#include "server/server.h"

struct tower_def_manager_s {
    tower_def_t *towerDefs;
    int numTowerDefs;
};

struct tower_manager_s {
    tower_state_t *towers;
    uint32_t *towerIDs; // Maps tower ID to index in towers array
    uint32_t *freeSlots; // Stack of free tower slots
    uint32_t numTowers;
    uint32_t numFreeSlots;
    uint32_t maxTowers;
    uint32_t nextTowerID;
    tower_def_manager_t *towerDefManager;
};

void tower_entity_think(const entity_manager_t *entityManager, entity_t *ent);
void tower_entity_update(const entity_manager_t *entityManager, entity_t *ent, float deltaTime);
void tower_entity_draw(const entity_manager_t *entityManager, entity_t *ent);
void tower_entity_destroy(const entity_manager_t *entityManager, entity_t *ent);

tower_manager_t *tower_init(tower_def_manager_t *defManager, const uint32_t maxTowers) {
    uint32_t i;
    tower_manager_t *towerManager = gfc_allocate_array(sizeof(tower_manager_t), 1);
    if (!towerManager) {
        return NULL;
    }

    towerManager->towers = gfc_allocate_array(sizeof(tower_state_t), maxTowers);
    towerManager->towerIDs = gfc_allocate_array(sizeof(uint32_t), maxTowers);
    towerManager->freeSlots = gfc_allocate_array(sizeof(uint32_t), maxTowers);
    towerManager->numTowers = 0;
    towerManager->numFreeSlots = maxTowers;
    towerManager->maxTowers = maxTowers;
    towerManager->nextTowerID = 0;
    towerManager->towerDefManager = defManager;

    for (i = 0; i < maxTowers; i++) {
        towerManager->freeSlots[i] = maxTowers - 1 - i; // Fill free slots stack
        towerManager->towerIDs[i] = UINT32_MAX; // Mark all IDs as invalid
    }

    return towerManager;
}

void tower_close(const tower_manager_t *towerManager) {
    if (towerManager->towers) free(towerManager->towers);
    if (towerManager->towerIDs) free(towerManager->towerIDs);
    if (towerManager->freeSlots) free(towerManager->freeSlots);

    if (towerManager->towerDefManager) {
        if (towerManager->towerDefManager->towerDefs) free(towerManager->towerDefManager->towerDefs);
        free(towerManager->towerDefManager);
    }
}

tower_def_manager_t * tower_load_defs(const struct def_manager_s *defManager, char *filePath) {
    tower_def_manager_t *towerDefManager;
    def_data_t *data, *towerJson, *towersArray;
    def_data_t *costArray, *valueArray, *weaponJson, *weaponDefJson, *modelDefJson;
    tower_def_t *def;
    tower_weapon_def_t *weaponDef;
    const char * str;
    int i, j, k, flag;

    towerDefManager = gfc_allocate_array(sizeof(tower_def_manager_t), 1);
    if (!towerDefManager) {
        log_error("Failed to allocate memory for tower definition manager");
        return NULL;
    }

    data = def_load(defManager, filePath);
    if (!data) {
        log_error("Failed to load tower definitions from file: %s", filePath);
        free(towerDefManager);
        return NULL;
    }

    towersArray = def_data_get_array(data, "towers");

    def_data_array_get_count(towersArray, &towerDefManager->numTowerDefs);
    towerDefManager->towerDefs = gfc_allocate_array(sizeof(tower_def_t), towerDefManager->numTowerDefs);
    for (i = 0; i < towerDefManager->numTowerDefs; i++) {
        towerJson = def_data_array_get_nth(towersArray, i);
        def = &towerDefManager->towerDefs[i];

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

        str = def_data_get_string(towerJson, "type");
        if (!str) {
            log_error("Missing type for tower definition at index %u", i);
            continue;
        }
        tower_type_from_string(str, &def->type);

        sj_object_get_float(towerJson, "size", &def->size);

        valueArray = def_data_get_array(towerJson, "maxHealth");
        for (j = 0; j < TOWER_MAX_LEVEL; j++) {
            sj_get_float_value(def_data_array_get_nth(valueArray, j), &def->maxHealth[j]);
        }

        weaponJson = def_data_get_array(towerJson, "weapons");
        if (weaponJson) {
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

                str = def_data_get_string(weaponDefJson, "projectileSprite");
                if (!str || strlen(str) >= sizeof(weaponDef->projectileSprite)) {
                    log_error("Invalid or missing projectileSprite for weapon definition at index %u of tower definition at index %u", j, i);
                    continue;
                }
                strncpy(weaponDef->projectileSprite, str, sizeof(weaponDef->projectileSprite) - 1);
                weaponDef->projectileSprite[sizeof(weaponDef->projectileSprite) - 1] = '\0';

                weaponDef->flags = 0;
                def_data_get_int(weaponDefJson, "directional", &flag);
                if (flag) weaponDef->flags |= TOWER_WEAPON_FLAG_DIRECTIONAL;
                def_data_get_int(weaponDefJson, "piercing", &flag);
                if (flag) weaponDef->flags |= TOWER_WEAPON_FLAG_PIERCING;
                def_data_get_int(weaponDefJson, "areaEffect", &flag);
                if (flag) weaponDef->flags |= TOWER_WEAPON_FLAG_AREA_EFFECT;
                valueArray = def_data_get_array(weaponDefJson, "direction");
                sj_get_float_value(def_data_array_get_nth(valueArray, 0), &weaponDef->direction.x);
                sj_get_float_value(def_data_array_get_nth(valueArray, 1), &weaponDef->direction.y);
            }
        }

        modelDefJson = def_data_get_obj(towerJson, "model");
        str = def_data_get_string(modelDefJson, "base");
        if (!str || strlen(str) >= sizeof(def->modelDef.baseSpritePath)) {
            log_error("Invalid or missing spritePath for tower definition at index %u", i);
            continue;
        }

        strncpy(def->modelDef.baseSpritePath, str, sizeof(def->modelDef.baseSpritePath) - 1);
        def->modelDef.baseSpritePath[sizeof(def->modelDef.baseSpritePath) - 1] = '\0';

        str = def_data_get_string(modelDefJson, "head");
        if (str) {
            if (!str || strlen(str) >= sizeof(def->modelDef.weaponSpritePath)) {
                log_error("Invalid or missing modelDef.baseSpritePath for tower definition at index %u", i);
                continue;
            }

            strncpy(def->modelDef.weaponSpritePath, str, sizeof(def->modelDef.weaponSpritePath) - 1);
            def->modelDef.weaponSpritePath[sizeof(def->modelDef.weaponSpritePath) - 1] = '\0';
        }

        def->index = i;

        costArray = def_data_get_array(towerJson, "cost");
        for (j = 0; j < TOWER_MAX_LEVEL; j++) {
            valueArray = def_data_array_get_nth(costArray, j);
            if (g_game.role == GAME_ROLE_SERVER) {
                def->cost[j][0].def = item_def_get(g_game.itemDefManager, "wood"); //TODO: don't hardcode this
                def->cost[j][1].def = item_def_get(g_game.itemDefManager, "stone"); //TODO: don't hardcode this
                def->cost[j][2].def = item_def_get(g_game.itemDefManager, "gold"); //TODO: don't hardcode this
            } else {
                def->cost[j][0].def = item_def_get(g_game.itemDefManager, "wood"); //TODO: don't hardcode this
                def->cost[j][1].def = item_def_get(g_game.itemDefManager, "stone"); //TODO: don't hardcode this
                def->cost[j][2].def = item_def_get(g_game.itemDefManager, "gold"); //TODO: don't hardcode this
            }
            sj_get_uint32_value(def_data_array_get_nth(valueArray, 0), &def->cost[j][0].quantity);
            sj_get_uint32_value(def_data_array_get_nth(valueArray, 1), &def->cost[j][1].quantity);
            sj_get_uint32_value(def_data_array_get_nth(valueArray, 2), &def->cost[j][2].quantity);
        }
    }

    return towerDefManager;
}

const tower_def_t *tower_def_get(const tower_manager_t *towerManager, const char *name) {
    int i;
    for (i = 0; i < towerManager->towerDefManager->numTowerDefs; i++) {
        if (strcmp(towerManager->towerDefManager->towerDefs[i].name, name) == 0) {
            return &towerManager->towerDefManager->towerDefs[i];
        }
    }
    return NULL; // Not found
}

const tower_def_t * tower_def_get_by_index(const tower_manager_t *towerManager, const int index) {
    if (index >= towerManager->towerDefManager->numTowerDefs) {
        return NULL; // Index out of bounds
    }
    return &towerManager->towerDefManager->towerDefs[index];
}

entity_t *tower_create_by_name(const entity_manager_t *entityManager, tower_manager_t *towerManager, const char* name, const GFC_Vector2D position) {
    const tower_def_t *def = tower_def_get(towerManager, name);
    if (!def) {
        log_error("Tower definition not found for name: %s", name);
        return NULL;
    }
    return tower_create_by_def(entityManager, towerManager, def, position);
}

entity_t *tower_create_by_def(const entity_manager_t *entityManager, tower_manager_t *towerManager,
                                    const tower_def_t *def, const GFC_Vector2D position) {
    entity_t *tower = tower_place(entityManager, towerManager, def, position, towerManager->nextTowerID++);
    if (!tower) {
        log_error("Failed to create tower at position (%f, %f)", position.x, position.y);
        return NULL;
    }

    return tower;
}

entity_t *tower_place(const entity_manager_t *entityManager, tower_manager_t *towerManager, const tower_def_t *def,
                            const GFC_Vector2D position, const uint32_t id) {
    tower_state_t * tower;
    entity_t *ent;
    if (towerManager->numFreeSlots == 0) {
        log_error("No free tower slots available");
        return NULL;
    }

    uint32_t slotIndex = towerManager->freeSlots[--towerManager->numFreeSlots];
    tower = &towerManager->towers[slotIndex];

    tower->id = id;
    towerManager->towerIDs[tower->id] = slotIndex; // Map tower ID to index in towers array
    tower->health = def->maxHealth[0];
    tower->def = def;

    // TODO: create entity based on tower definition's modelDef and assign to tower->entity
    ent = entity_new(entityManager);
    if (!ent) {
        log_error("Failed to create entity for tower");
        towerManager->freeSlots[towerManager->numFreeSlots++] = slotIndex; // Return slot to free stack
        return NULL;
    }

    ent->think = tower_entity_think;
    ent->update = tower_entity_update;
    ent->draw = tower_entity_draw;
    ent->destroy = tower_entity_destroy;
    ent->data = tower;
    tower->entity = ent;

    if (g_game.role == GAME_ROLE_CLIENT) {
        char spritePath[256];
        snprintf(spritePath, sizeof(spritePath), def->modelDef.baseSpritePath, tower->level + 1);
        tower->baseSprite = gf2d_sprite_load_image(spritePath);
        snprintf(spritePath, sizeof(spritePath), def->modelDef.weaponSpritePath, tower->level + 1);
        tower->weaponSprite = gf2d_sprite_load_image(spritePath);
    }

    tower->worldPos = world_pos_tile_snap(g_game.world, position);
    ent->position = tower->worldPos;
    world_add_entity(g_game.world, ent);

    ent->layers = ENT_LAYER_TOWER | ENT_LAYER_PLAYER;
    ent->boundingBox = gfc_rect(0, 0, def->size * TILE_SIZE, def->size * TILE_SIZE);

    return ent;
}

entity_t * tower_get_by_id(tower_manager_t *towerManager, const uint32_t id) {
    uint32_t index;
    if (id >= towerManager->maxTowers) {
        return NULL; // Invalid ID
    }
    index = towerManager->towerIDs[id];
    if (index == UINT32_MAX || index >= towerManager->maxTowers) {
        return NULL; // ID not in use or index out of bounds
    }
    return towerManager->towers[index].entity;
}

void tower_destroy(tower_manager_t *towerManager, entity_t *entity) {
    uint32_t index;
    tower_state_t *tower;

    if (!entity || !entity->data) return;

    tower = (tower_state_t *)entity->data;
    index = towerManager->towerIDs[tower->id];
    if (index == UINT32_MAX || index >= towerManager->maxTowers) {
        log_error("Attempted to destroy tower with invalid ID: %u", tower->id);
        return;
    }
    towerManager->freeSlots[towerManager->numFreeSlots++] = index;
    tower->id = UINT32_MAX;

    if (tower->entity) {
        entity_free(NULL, tower->entity);
        tower->entity = NULL;
    }
}

void tower_type_from_string(const char *str, tower_type_t *outType) {
    if (strcmp(str, "defensive") == 0) {
        *outType = TOWER_TYPE_DEFENSIVE;
    } else if (strcmp(str, "production") == 0) {
        *outType = TOWER_TYPE_PRODUCTION;
    } else if (strcmp(str, "passive") == 0) {
        *outType = TOWER_TYPE_PASSIVE;
    } else if (strcmp(str, "stash") == 0) {
        *outType = TOWER_TYPE_STASH;
    }
}

int tower_try_shoot(entity_t *entity, const float deltaTime) {
    if (!entity || !entity->data) return 0;
    tower_state_t *tower = (tower_state_t *)entity->data;

    if (tower->def->numWeapons <= 0) return 0;

    if (tower->cooldown > 0) {
        tower->cooldown -= deltaTime;
        if (tower->cooldown < 0) tower->cooldown = 0;
    }

    if (tower->cooldown <= 0) {
        tower->cooldown = tower->def->weaponDefs[0].fireRate[tower->level]; // TODO: Handle multiple weapons and levels properly
        return 1; // Can shoot
    }

    return 0; // Still cooling down
}

void tower_shoot(const entity_manager_t *entityManager, entity_t *entity, int weaponIndex) {
    tower_state_t *tower;
    const tower_weapon_def_t *weaponDef;
    int level;
    if (!entity || !entity->data) return;

    tower = (tower_state_t *)entity->data;
    if (weaponIndex < 0 || weaponIndex >= tower->def->numWeapons) return;

    level = tower->level;
    weaponDef = &tower->def->weaponDefs[weaponIndex];
    projectile_spawn(
        entityManager,
        weaponDef->bulletSpeed[level],
        weaponDef->damage[level],
        weaponDef->range[level],
        weaponDef->direction,
        weaponDef->projectileSprite,
        tower
    );
}

void tower_shoot_all(const entity_manager_t *entityManager, entity_t *entity) {
    int i;
    tower_state_t *tower;
    if (!entity || !entity->data) return;

    tower = (tower_state_t *)entity->data;
    if (tower->def->numWeapons <= 0) return;
    for (i = 0; i < tower->def->numWeapons; i++) {
        tower_shoot(entityManager, entity, i);
    }
}

void tower_entity_draw_full(float size, GFC_Vector2D pos, Sprite *baseSprite, Sprite *weaponSprite) {
    GFC_Vector2D drawPos, headPos, centerPos;
    gfc_vector2d_sub(drawPos, pos, g_camera.position);
    gf2d_sprite_draw_image(baseSprite, drawPos);

    headPos.x = drawPos.x + (size * TILE_SIZE / 2) - ((float)weaponSprite->frame_w / 2);
    headPos.y = drawPos.y + (size * TILE_SIZE / 2) - ((float)weaponSprite->frame_h / 2);
    gf2d_sprite_draw_image(weaponSprite, headPos);
}

inventory_transaction_t * tower_get_cost_transaction(const tower_def_t *def, int level) {
    inventory_transaction_t *transaction = inventory_transaction_create(3, 0);
    if (!transaction) {
        return NULL;
    }

    transaction->items[0] = def->cost[level][0];
    transaction->items[1] = def->cost[level][1];
    transaction->items[2] = def->cost[level][2];

    return transaction;
}

void tower_entity_think(const entity_manager_t *entityManager, entity_t *ent) {
    if (!ent) return;
    tower_state_t *tower = (tower_state_t *)ent->data;
    if (!tower) return;

    // TODO: Implement tower thinking behavior, such as targeting enemies, deciding when to shoot, etc.
}

void tower_entity_update(const entity_manager_t *entityManager, entity_t *ent, float deltaTime) {
    if (!ent || ! ent->data) return;
    tower_state_t *tower = (tower_state_t *)ent->data;

    if (g_game.role == GAME_ROLE_SERVER) {
        if (tower_try_shoot(ent, deltaTime)) {

            s2c_tower_event_packet_t pkt;
            create_s2c_tower_event(&pkt, tower->id, TOWER_EVENT_SHOOT, *(uint64_t*) &ent->rotation);
            server_broadcast_packet(&g_server, &pkt, 0);

            tower_shoot_all(entityManager, ent);
        }
    }
}

void tower_entity_draw(const entity_manager_t *entityManager, entity_t *ent) {
    if (!ent || !ent->data) return;
    tower_state_t *tower = (tower_state_t *)ent->data;
    tower_entity_draw_full(tower->def->size, ent->position, tower->baseSprite, tower->weaponSprite);
}

void tower_entity_destroy(const entity_manager_t *entityManager, entity_t *ent) {
    if (!ent || !ent->data) return;
    tower_state_t *tower = (tower_state_t *)ent->data;

    if (tower->baseSprite) {
        gf2d_sprite_free(tower->baseSprite);
        tower->baseSprite = NULL;
    }
    if (tower->weaponSprite) {
        gf2d_sprite_free(tower->weaponSprite);
        tower->weaponSprite = NULL;
    }
}