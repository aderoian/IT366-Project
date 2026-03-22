#include "common/game/enemy.h"

#include "client/camera.h"
#include "client/gf2d_draw.h"
#include "client/gf2d_sprite.h"
#include "common/def.h"
#include "common/logger.h"
#include "common/game/collision.h"
#include "common/game/game.h"
#include "common/game/world/world.h"
#include "common/network/packet/definitions.h"
#include "common/network/packet/io.h"
#include "server/server.h"

extern uint8_t __DEBUG;

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

    return enemyDefManager;
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
    entity_t *ent;
    Sprite *bodySprite, *handsSprite = NULL;

    ent = entity_new(entityManager, entity_next_id((entity_manager_t *)entityManager));
    ent->position = pos;

    ent->layers = ENT_LAYER_ENEMY;
    ent->boundingBox = def->modelDef.boundingBox;

    world_add_entity(g_game.world, ent);
    enemy_state_t *state = gfc_allocate_array(sizeof(enemy_state_t), 1);
    state->def = def;
    state->health = def->maxHealth[0];

    if (g_game.role == GAME_ROLE_CLIENT) {
        char spritePath[256];
        snprintf(spritePath, sizeof(spritePath), def->modelDef.bodySpritePath, 1);
        state->bodySprite = gf2d_sprite_load_image(spritePath);

        snprintf(spritePath, sizeof(spritePath), def->modelDef.handsSpritePath, 1);
        state->handsSprite = gf2d_sprite_load_image(spritePath);\
    }

    state->targets = gfc_list_new();

    ent->data = state;

    ent->think = enemy_think;
    ent->update = enemy_update;
    ent->draw = enemy_draw;
    ent->destroy = enemy_destroy;
    ent->collidesWith = enemy_collides_with;

    return ent;
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

GFC_Vector2D enemy_move(entity_t *ent, float deltaTime) {
    GFC_Vector2D direction, moveDelta, newPosition;
    float speed;
    if (!ent || !ent->data) {
        return ent ? ent->position : gfc_vector2d(0, 0);
    }

    enemy_state_t *state = (enemy_state_t *)ent->data;
    world_t *world = g_game.world;
    speed = state->def->speed[state->level];

    gfc_vector2d_sub(direction, g_game.state.stashPosition, ent->position);
    gfc_vector2d_normalize(&direction);
    gfc_vector2d_scale(moveDelta, direction, speed * deltaTime);
    gfc_vector2d_add(newPosition, ent->position, moveDelta);

    if (newPosition.x < 0 || newPosition.y < 0 || newPosition.x >= world->size.x * CHUNK_TILE_SIZE * TILE_SIZE || newPosition.y >= world->size.y * CHUNK_TILE_SIZE * TILE_SIZE) {
        return gfc_vector2d(fmaxf(0, fminf(newPosition.x, world->size.x * CHUNK_TILE_SIZE * TILE_SIZE - 1)), fmaxf(0, fminf(newPosition.y, world->size.y * CHUNK_TILE_SIZE * TILE_SIZE - 1)));
    }

    while (!collision_check_world(world, ent, newPosition)) {
        gfc_vector2d_scale(moveDelta, moveDelta, 0.5f);
        gfc_vector2d_add(newPosition, ent->position, moveDelta);

        if (gfc_vector2d_magnitude(moveDelta) < 0.01f) {
            return ent->position; // Movement is too small, likely stuck, so give up
        }
    }

    world_move_entity(world, ent, newPosition);
    return newPosition;
}

void enemy_think(const entity_manager_t *entityManager, entity_t *ent) {
    GFC_Vector2D direction, rayCastEnd;
    if (!ent || !ent->data) {
        return;
    }

    if (g_game.role != GAME_ROLE_SERVER) {
        return; // Only run AI logic on the server
    }

    enemy_state_t *state = (enemy_state_t *)ent->data;

    state->attackTargetTimer -= g_game.deltaTime;
    state->attackCooldownTimer -= g_game.deltaTime;

    gfc_vector2d_sub(direction, g_game.state.stashPosition, ent->position);
    gfc_vector2d_normalize(&direction);
    ent->rotation = atan2f(direction.y, direction.x) * 180.0f / M_PI + 90.0f;

    if (state->attackTargetTimer <= 0) {
        gfc_vector2d_scale(direction, direction, state->def->range[state->level]);
        gfc_vector2d_add(rayCastEnd, ent->position, direction);

        gfc_list_clear(state->targets);

        collision_raycast_world(g_game.world, ent, ent->position, rayCastEnd, state->targets);

        // TODO: Filter targets
        state->attackTargetTimer = g_game.deltaTime * 5; // every 5 ticks
    }


}

void enemy_update(const entity_manager_t *entityManager, entity_t *ent, float deltaTime) {
    GFC_Vector2D newPosition, diff;
    if (!ent || !ent->data) {
        return;
    }

    if (g_game.role != GAME_ROLE_SERVER) {
        return; // Only run AI logic on the server
    }

    enemy_state_t *state = (enemy_state_t *)ent->data;

    newPosition = enemy_move(ent, deltaTime);
    gfc_vector2d_sub(diff, ent->position, newPosition);
    if (newPosition.x != ent->position.x || newPosition.y != ent->position.y) {
        state->dirtyFlags |= ENEMY_DIRTY_POSITION;
    }
    ent->position = newPosition;

    if (state->attackCooldownTimer <= 0 && gfc_list_count(state->targets) > 0) {
        // TODO: Implement attack logic (e.g., apply damage to target, play attack animation, etc.)
        state->attackCooldownTimer = state->def->attackCooldown[state->level];
        state->dirtyFlags |= ENEMY_DIRTY_ATTACK;
    }

    if (state->dirtyFlags) {
        s2c_enemy_snapshot_packet_t *pkt = gfc_allocate_array(sizeof(s2c_enemy_snapshot_packet_t), 1);
        enemy_snapshot_data_t eventData;
        if (state->dirtyFlags & ENEMY_DIRTY_POSITION) {
            eventData.moveData.xPos = ent->position.x;
            eventData.moveData.yPos = ent->position.y;
            eventData.moveData.rotation = ent->rotation;
        }
        create_s2c_enemy_snapshot(pkt, ent->id, state->dirtyFlags & ENEMY_DIRTY_POSITION ? ENEMY_EVENT_MOVE : ENEMY_EVENT_ATTACK, &eventData);
        server_broadcast_packet_batch(&g_server, pkt);
    }

    state->dirtyFlags = 0;
}

void enemy_draw(const entity_manager_t *entityManager, entity_t *ent) {
    GFC_Vector2D position, centerPos, heldItemOffset, itemCenterPos;
    Sprite *headSprite, *handsSprite;
    if (!ent || !ent->data) {
        return;
    }

    enemy_state_t *state = (enemy_state_t *)ent->data;

    gfc_vector2d_sub(position, ent->position, g_camera.position);

    headSprite = state->bodySprite;
    handsSprite = state->handsSprite;
    centerPos = gfc_vector2d(headSprite->frame_w * 0.5f, headSprite->frame_h * 0.5f);
    heldItemOffset = gfc_vector2d(handsSprite->frame_w * 0.5f, handsSprite->frame_h);

    gf2d_sprite_draw(handsSprite, position, NULL, &heldItemOffset, &ent->rotation, NULL, NULL, 0);
    gf2d_sprite_draw(headSprite, position,NULL, &centerPos, NULL, NULL, NULL, 0);

    if (__DEBUG) {
        GFC_Rect bounding = state->def->modelDef.boundingBox;
        bounding.x -= g_camera.position.x;
        bounding.y -= g_camera.position.y;
        gf2d_draw_rect(bounding, GFC_COLOR_DARKBLUE);
    }
}

void enemy_destroy(const entity_manager_t *entityManager, entity_t *ent) {
    if (!ent || !ent->data) {
        return;
    }

    enemy_state_t *state = (enemy_state_t *)ent->data;

    gfc_list_delete(state->targets);
    gf2d_sprite_free(state->bodySprite);
    gf2d_sprite_free(state->handsSprite);

    if (g_game.role == GAME_ROLE_SERVER) {
        s2c_enemy_snapshot_packet_t *pkt = gfc_allocate_array(sizeof(s2c_enemy_snapshot_packet_t), 1);
        enemy_snapshot_data_t eventData;
        create_s2c_enemy_snapshot(pkt, ent->id, ENEMY_EVENT_DESPAWN, &eventData);
        server_broadcast_packet_batch(&g_server, pkt);
    }
}

uint32_t enemy_collides_with(entity_t *ent, entity_t *other) {
    if (!ent || !other || !ent->data) {
        return 0;
    }

    if (other->layers & ENT_LAYER_TOWER) {
        return COLLISION_SOLID;
    }

    return COLLISION_NONE;
}