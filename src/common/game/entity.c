#include "../../../include/common/game/entity.h"

#include "client/animation.h"
#include "client/gf2d_sprite.h"

#include "client/camera.h"
#include "common/logger.h"

typedef struct EntityManager_S {
    Entity *ents;
    uint32_t maxEnts;
} EntityManager;

static EntityManager ent_manager = {0};

void entity_init(const uint32_t maxEnts) {
    ent_manager.ents = gfc_allocate_array(sizeof(Entity), maxEnts);
    ent_manager.maxEnts = maxEnts;
}

void entity_close(void) {
    if (ent_manager.ents) free(ent_manager.ents);
}

Entity *entity_new(void) {
    uint32_t i;
    Entity* ent;
    if (!ent_manager.ents) 
        return NULL;

    for (i = 0; i < ent_manager.maxEnts; i++) {
        ent = &ent_manager.ents[i];
        if (ent->_inUse > 0) continue;

        ent->_inUse = 1;
        ent->draw = entity_draw;
        ent->scale = gfc_vector2d(1, 1);
        ent->layers = 0xFFFF;

        return ent;
    }

    return NULL;
}

Entity *entity_new_animated(void) {
    Entity *ent = entity_new();
    if (!ent) return NULL;

    ent->flags |= ENT_FLAG_ANIMATED;
    ent->draw = entity_draw_animated;
    ent->update = entity_update_animated;

    return ent;
}

void entity_free(Entity* ent) {
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

void entity_think_all(void) {
    uint32_t i;
    Entity *ent;
    for (i = 0; i < ent_manager.maxEnts; i++) {
        ent = &ent_manager.ents[i];
        if (ent->_inUse == 0 || !ent->think) continue;
        ent->think(ent);
    }
}
void entity_update_all(const float deltaTime) {
    uint32_t i;
    Entity *ent;
    for (i = 0; i < ent_manager.maxEnts; i++) {
        ent = &ent_manager.ents[i];
        if (ent->_inUse == 0 || !ent->update) continue;
        ent->update(ent, deltaTime);
    }
}

void entity_update_animated(Entity *ent, float deltaTime) {
    if (!ent) return;
    if (!(ent->flags & ENT_FLAG_ANIMATED)) return;

    // Assuming model is of type AnimatedSprite when ENT_FLAG_ANIMATED is set
    AnimatedSprite *animatedSprite = (AnimatedSprite *)ent->model;
    if (!animatedSprite) return;

    animation_state_update(animatedSprite->state, deltaTime);
}

void entity_draw(Entity *ent) {
    GFC_Vector2D position;
    if (!ent) return;

    gfc_vector2d_sub(position, ent->position, g_camera.position);
    gf2d_sprite_draw_image(ent->model, position);
}

void entity_draw_animated(Entity *ent) {
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

void entity_draw_all(void) {
    uint32_t i;
    Entity *ent;
    for (i = 0; i < ent_manager.maxEnts; i++) {
        ent = &ent_manager.ents[i];
        if (ent->_inUse == 0 || !ent->draw) continue;
        ent->draw(ent);
    }
}