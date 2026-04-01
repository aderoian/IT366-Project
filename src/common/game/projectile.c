#include "common/logger.h"
#include "common/game/entity.h"
#include "common/game/projectile.h"

#include "client/camera.h"
#include "client/client.h"
#include "common/game/tower.h"

#include "../../../include/common/render/gf2d_sprite.h"
#include "common/game/collision.h"
#include "common/game/enemy.h"
#include "common/game/game.h"
#include "server/server.h"

extern uint8_t __INF_DAMAGE;

void projectile_think(const entity_manager_t *entityManager, entity_t *ent);
void projectile_update(const entity_manager_t *entityManager, entity_t *ent, float deltaTime);
void projectile_draw(const entity_manager_t *entityManager, entity_t *ent);
void projectile_destroy(const entity_manager_t *entityManager, entity_t *ent);
uint32_t projectile_collides_with(entity_t *ent, entity_t *other);
uint32_t projectile_on_collide(entity_t *ent, entity_t *other, uint32_t collisionType);

int projectile_spawn(const entity_manager_t *entityManager, const float speed, const float damage, const float range, uint8_t areaDamage,
                        const GFC_Vector2D direction, const char *spriteModel, struct tower_state_s *sourceTower) {
    projectile_state_t *projectile;
    entity_t *ent;

    if (!sourceTower) {
        log_error("Source tower is NULL when trying to spawn projectile");
        return -1;
    }

    ent = entity_new(entityManager, entity_next_id(entityManager));
    if (!ent) {
        log_error("Failed to create new entity for projectile");
        return -1;
    }

    // Initialize projectile state
    projectile = (projectile_state_t *)gfc_allocate_array(sizeof(projectile_state_t), 1);
    if (!projectile) {
        log_error("Failed to allocate memory for projectile state");
        entity_free(entityManager, ent);
        return -1;
    }

    projectile->speed = speed;
    projectile->damage = damage;
    projectile->range = range;
    projectile->areaDamage = areaDamage;
    gfc_vector2d_copy(projectile->direction, direction);
    projectile->distanceTraveled.x = 0; projectile->distanceTraveled.y = 0;
    projectile->sourceTower = sourceTower;
    projectile->entity = ent;

    world_add_entity(g_game.world, ent);

    // Set up the entity's properties
    ent->think = projectile_think;
    ent->update = projectile_update;
    ent->draw = projectile_draw;
    ent->destroy = projectile_destroy;
    ent->collidesWith = projectile_collides_with;
    ent->onCollide = projectile_on_collide;

    ent->layers = ENT_LAYER_PROJECTILE;
    ent->boundingBox = gfc_rect(-12, -12, 24, 24); // Example bounding box size for projectile, can be adjusted based on sprite

    // Position the entity at the source tower's location
    ent->position = sourceTower->worldPos;
    ent->rotation = gfc_vector2d_angle(direction) * 180.0f / M_PI;
    ent->data = projectile;

    if (g_game.role == GAME_ROLE_CLIENT) {
        ent->model = gf2d_sprite_load_image(spriteModel);
    }

    return 0;
}

void projectile_think(const entity_manager_t *entityManager, entity_t *ent) {
    if (!ent || !ent->data) {
        log_error("Invalid entity or missing projectile data in projectile_think");
        return;
    }

    collision_check_world(g_game.world, ent, ent->position);
}

void projectile_update(const entity_manager_t *entityManager, entity_t *ent, const float deltaTime) {
    GFC_Vector2D movement, pos;
    if (!ent || !ent->data) {
        log_error("Invalid entity or missing projectile data in projectile_update");
        return;
    }

    projectile_state_t *projectile = (projectile_state_t *)ent->data;

    // Calculate movement based on speed and direction
    gfc_vector2d_scale(movement, projectile->direction, projectile->speed * deltaTime);
    gfc_vector2d_add(pos, ent->position, movement);

    if (g_game.role == GAME_ROLE_SERVER) {
        if (!world_move_entity(g_game.world, ent, pos)) {
            log_error("Failed to move projectile entity in world");
            return;
        }
    }

    ent->position = pos; // Update position after successful move

    // Check if the projectile has exceeded its range
    gfc_vector2d_add(projectile->distanceTraveled, projectile->distanceTraveled, movement);
    if (gfc_vector2d_magnitude(projectile->distanceTraveled) >= projectile->range) {
        ent->think = entity_free;
    }
}

void projectile_draw(const entity_manager_t *entityManager, entity_t *ent) {
    GFC_Vector2D position;
    if (!ent || !ent->data) {
        log_error("Invalid entity or missing projectile data in projectile_draw");
        return;
    }

    gfc_vector2d_sub(position, ent->position, g_camera.position);
    gf2d_sprite_draw_centered(ent->model, position, NULL, NULL, &ent->rotation, NULL, NULL, 0);
}

uint32_t projectile_collides_with(entity_t *ent, entity_t *other) {
    if (!ent || !other || !ent->data) {
        return COLLISION_NONE;
    }

    if (other->layers & ENT_LAYER_ENEMY) {
        return COLLISION_EVENT; // Collides with enemies
    }

    return COLLISION_NONE; // No collision
}

uint32_t projectile_on_collide(entity_t *ent, entity_t *other, uint32_t collisionType) {
    int i;
    if (!ent || !other || !ent->data) {
        return 0;
    }

    projectile_state_t *projectile = (projectile_state_t *)ent->data;

    if (collisionType & COLLISION_EVENT) {
        // Apply damage to the enemy
        if (other->data && g_game.role == GAME_ROLE_SERVER) {
            enemy_state_t *enemy = (enemy_state_t *)other->data;

            if (projectile->areaDamage) {
                GFC_List *enemyList = collision_get_entities_in_range(g_game.world, ent->position, projectile->range, ENT_LAYER_ENEMY);
                for (i = 0; i < gfc_list_count(enemyList); i++) {
                    entity_t *areaEnemyEnt = (entity_t *)gfc_list_nth(enemyList, i);
                    if (!areaEnemyEnt || !areaEnemyEnt->data) {
                        continue;
                    }
                    enemy_state_t *areaEnemy = (enemy_state_t *)areaEnemyEnt->data;
                    if (__INF_DAMAGE) {
                        areaEnemy->health = 0; // Instantly kill the enemy for testing purposes
                    } else {
                        areaEnemy->health -= projectile->damage;
                    }
                    areaEnemy->dirtyFlags |= ENEMY_DIRTY_HEALTH; // Mark enemy health as dirty to trigger update
                }
            } else {
                // If not area damage, only apply to the first enemy hit
                if (__INF_DAMAGE) {
                    enemy->health = 0; // Instantly kill the enemy for testing purposes
                }
                enemy->health -= projectile->damage;
            }

            enemy->dirtyFlags |= ENEMY_DIRTY_HEALTH; // Mark enemy health as dirty to trigger update
        }

        // Destroy the projectile after hitting an enemy
        ent->think = entity_free;
    }

    return 1;
}

void projectile_destroy(const entity_manager_t *entityManager, entity_t *ent) {
    if (!ent || !ent->data) {
        return;
    }

    world_remove_entity(g_game.world, ent);
}