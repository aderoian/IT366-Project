#include "common/logger.h"
#include "common/game/entity.h"
#include "common/game/projectile.h"
#include "common/game/tower.h"

#include "client/gf2d_sprite.h"
#include "common/game/game.h"

void projectile_think(Entity *ent);
void projectile_update(Entity *ent, float deltaTime);

int projectile_spawn(float speed, float damage, float range, GFC_Vector2D direction, const char *spriteModel, struct tower_state_s *sourceTower) {
    projectile_state_t *projectile;
    Entity *ent;

    if (!sourceTower) {
        log_error("Source tower is NULL when trying to spawn projectile");
        return -1;
    }

    ent = entity_new();
    if (!ent) {
        log_error("Failed to create new entity for projectile");
        return -1;
    }

    // Initialize projectile state
    projectile = (projectile_state_t *)gfc_allocate_array(sizeof(projectile_state_t), 1);
    if (!projectile) {
        log_error("Failed to allocate memory for projectile state");
        entity_free(ent);
        return -1;
    }

    projectile->speed = speed;
    projectile->damage = damage;
    projectile->range = range;
    gfc_vector2d_copy(projectile->direction, direction);
    projectile->distanceTraveled.x = 0; projectile->distanceTraveled.y = 0;
    projectile->sourceTower = sourceTower;
    projectile->entity = ent;

    // Set up the entity's properties
    ent->think = projectile_think;
    ent->update = projectile_update;

    // Position the entity at the source tower's location
    ent->position = sourceTower->worldPos;
    ent->data = projectile;

    if (g_game.isLocal) {
        ent->model = gf2d_sprite_load_image(spriteModel);
    }

    return 0;
}

void projectile_think(Entity *ent) {
    // Currently, the projectile does not have any specific thinking behavior.
    // This function can be expanded in the future to handle things like homing behavior, collision detection, etc.
}

void projectile_update(Entity *ent, const float deltaTime) {
    GFC_Vector2D movement;
    if (!ent || !ent->data) {
        log_error("Invalid entity or missing projectile data in projectile_update");
        return;
    }

    projectile_state_t *projectile = (projectile_state_t *)ent->data;

    // Calculate movement based on speed and direction
    gfc_vector2d_scale(movement, projectile->direction, projectile->speed * deltaTime);
    gfc_vector2d_add(ent->position, ent->position, movement);
    gfc_vector2d_add(projectile->distanceTraveled, projectile->distanceTraveled, movement);

    // Check if the projectile has exceeded its range
    if (gfc_vector2d_magnitude(projectile->distanceTraveled) >= projectile->range) {
        ent->think = entity_free;
    }
}