#include "entity.h"

#include "animation.h"

typedef struct EntityManager_S {
    Entity *ents;
    uint32_t maxEnts;
} EntityManager;

static EntityManager ent_manager = {0};

void entity_init(uint32_t maxEnts) {
    ent_manager.ents = gfc_allocate_array(sizeof(Entity), maxEnts);
    ent_manager.maxEnts = maxEnts;
}

void entity_close() {
    if (ent_manager.ents) free(ent_manager.ents);
}

Entity *entity_new() {
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

Entity *entity_new_animated() {
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

    memset(ent, 0, sizeof(Entity));
}

void entity_draw(Entity *ent) {
    if (!ent) return;
    gf2d_sprite_draw_image(ent->model, ent->position);
}

void entity_think_all() {
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

void entity_draw_all() {
    uint32_t i;
    Entity *ent;
    for (i = 0; i < ent_manager.maxEnts; i++) {
        ent = &ent_manager.ents[i];
        if (ent->_inUse == 0 || !ent->draw) continue;
        ent->draw(ent);
    }
}

void entity_draw_animated(Entity *ent) {
    if (!ent) return;
    if (!(ent->flags & ENT_FLAG_ANIMATED)) return;

    // Assuming model is of type AnimatedSprite when ENT_FLAG_ANIMATED is set
    AnimatedSprite *animatedSprite = (AnimatedSprite *)ent->model;
    if (!animatedSprite) return;

    gf2d_sprite_draw(
        animatedSprite->sprite,
        ent->position,
        &ent->scale,
        NULL,
        &ent->rotation,
        NULL,
        NULL,
        animation_state_getCurrentFrame(animatedSprite->state)
    );
}

void entity_update_animated(Entity *ent, float deltaTime) {
    if (!ent) return;
    if (!(ent->flags & ENT_FLAG_ANIMATED)) return;

    // Assuming model is of type AnimatedSprite when ENT_FLAG_ANIMATED is set
    AnimatedSprite *animatedSprite = (AnimatedSprite *)ent->model;
    if (!animatedSprite) return;

    animation_state_update(animatedSprite->state, deltaTime);
}