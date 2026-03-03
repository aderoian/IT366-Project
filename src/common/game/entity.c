#include "../../../include/common/game/entity.h"

#include "client/animation.h"
#include "client/gf2d_sprite.h"

#include "client/camera.h"
#include "common/logger.h"

struct entity_manager_s {
    Entity *ents;
    size_t maxEnts;
};

entity_manager_t *entity_init(const size_t maxEnts) {
    entity_manager_t *manager = malloc(sizeof(entity_manager_t));
    if (!manager) {
        log_error("Failed to allocate memory for entity manager");
        return NULL;
    }

    manager->ents = gfc_allocate_array(sizeof(Entity), maxEnts);
    if (!manager->ents) {
        free(manager);
        log_error("Failed to allocate memory for entities");
        return NULL;
    }

    manager->maxEnts = maxEnts;
    return manager;
}

void entity_close(const entity_manager_t *manager) {
    if (manager->ents) free(manager->ents);
}

Entity *entity_new(const entity_manager_t *manager) {
    size_t i;
    Entity* ent;
    if (!manager->ents) 
        return NULL;

    for (i = 0; i < manager->maxEnts; i++) {
        ent = &manager->ents[i];
        if (ent->_inUse > 0) continue;

        ent->_inUse = 1;
        ent->draw = entity_draw;
        ent->scale = gfc_vector2d(1, 1);
        ent->layers = 0xFFFF;

        return ent;
    }

    return NULL;
}

Entity *entity_new_animated(const entity_manager_t *manager) {
    Entity *ent = entity_new(manager);
    if (!ent) return NULL;

    ent->flags |= ENT_FLAG_ANIMATED;
    ent->draw = entity_draw_animated;
    ent->update = entity_update_animated;

    return ent;
}

void entity_free(const entity_manager_t *entityManager, Entity* ent) {
    if (!ent) return;

    if (ent->flags & ENT_FLAG_ANIMATED) {
        // Assuming model is of type AnimatedSprite when ENT_FLAG_ANIMATED is set
        AnimatedSprite *animatedSprite = (AnimatedSprite *)ent->model;
        if (animatedSprite) {
            animation_sprite_freeSprite(animatedSprite);
        }
    } else {
        if (ent->model) gf2d_sprite_free(ent->model);
    }

    if (ent->data) free(ent->data);

    memset(ent, 0, sizeof(Entity));
}

void entity_think_all(const entity_manager_t *manager) {
    size_t i;
    Entity *ent;
    for (i = 0; i < manager->maxEnts; i++) {
        ent = &manager->ents[i];
        if (ent->_inUse == 0 || !ent->think) continue;
        ent->think(manager, ent);
    }
}
void entity_update_all(const entity_manager_t *manager, const float deltaTime) {
    size_t i;
    Entity *ent;
    for (i = 0; i < manager->maxEnts; i++) {
        ent = &manager->ents[i];
        if (ent->_inUse == 0 || !ent->update) continue;
        ent->update(manager, ent, deltaTime);
    }
}

void entity_update_animated(const entity_manager_t *entityManager, Entity *ent, float deltaTime) {
    if (!ent) return;
    if (!(ent->flags & ENT_FLAG_ANIMATED)) return;

    // Assuming model is of type AnimatedSprite when ENT_FLAG_ANIMATED is set
    AnimatedSprite *animatedSprite = (AnimatedSprite *)ent->model;
    if (!animatedSprite) return;

    animation_state_update(animatedSprite->state, deltaTime);
}

void entity_draw(const entity_manager_t *entityManager, Entity *ent) {
    GFC_Vector2D position;
    if (!ent) return;

    gfc_vector2d_sub(position, ent->position, g_camera.position);
    gf2d_sprite_draw_image(ent->model, position);
}

void entity_draw_animated(const entity_manager_t *entityManager, Entity *ent) {
    GFC_Vector2D position;
    if (!ent) return;
    if (!(ent->flags & ENT_FLAG_ANIMATED)) return;

    // Assuming model is of type AnimatedSprite when ENT_FLAG_ANIMATED is set
    AnimatedSprite *animatedSprite = (AnimatedSprite *)ent->model;
    if (!animatedSprite) return;

    gfc_vector2d_sub(position, ent->position, g_camera.position);

    gf2d_sprite_draw(
        animatedSprite->sprite,
        position,
        &ent->scale,
        NULL,
        &ent->rotation,
        NULL,
        NULL,
        animation_state_getCurrentFrame(animatedSprite->state)
    );
}

void entity_draw_all(const entity_manager_t *manager) {
    size_t i;
    Entity *ent;
    for (i = 0; i < manager->maxEnts; i++) {
        ent = &manager->ents[i];
        if (ent->_inUse == 0 || !ent->draw) continue;
        ent->draw(manager, ent);
    }
}