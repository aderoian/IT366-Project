#include "../../../include/common/game/entity.h"

#include "client/animation.h"
#include "client/gf2d_sprite.h"

#include "client/camera.h"
#include "client/gf2d_draw.h"
#include "common/logger.h"
#include "common/game/game.h"

extern uint8_t __DEBUG;

struct entity_manager_s {
    entity_t *ents;
    uint64_t *idToSlot;
    uint32_t maxEnts;
    int64_t maxIdSlots;
    int64_t nextId;
};

entity_manager_t *entity_init(const uint32_t maxEnts) {
    entity_manager_t *manager = malloc(sizeof(entity_manager_t));
    if (!manager) {
        log_error("Failed to allocate memory for entity manager");
        return NULL;
    }

    manager->ents = gfc_allocate_array(sizeof(entity_t), maxEnts);
    if (!manager->ents) {
        free(manager);
        log_error("Failed to allocate memory for entities");
        return NULL;
    }

    manager->idToSlot = calloc(maxEnts, sizeof(uint64_t));
    if (!manager->idToSlot) {
        free(manager->ents);
        free(manager);
        log_error("Failed to allocate memory for ID to slot mapping");
        return NULL;
    }

    manager->maxEnts = maxEnts;
    manager->maxIdSlots = maxEnts;
    manager->nextId = 1; // Start IDs from 1 to avoid using
    return manager;
}

void entity_close(const entity_manager_t *manager) {
    if (manager->ents) free(manager->ents);
}

entity_t *entity_new(entity_manager_t *manager, const int64_t id) {
    uint32_t i;
    entity_t* ent;
    if (!manager->ents) 
        return NULL;

    for (i = 0; i < manager->maxEnts; i++) {
        ent = &manager->ents[i];
        if (ent->_inUse > 0) continue;

        ent->_inUse = 1;
        ent->id = id;
        ent->draw = entity_draw;
        ent->scale = gfc_vector2d(1, 1);
        ent->layers = 0xFFFF;

        if (id < 0) {
            return ent; // Negative IDs are not mapped, client side
        }

        if (id >= manager->maxIdSlots) {
            uint64_t newMaxIdSlots = manager->maxIdSlots * 2;
            uint64_t *newIdToSlot = malloc(sizeof(uint64_t) * newMaxIdSlots);
            if (!newIdToSlot) {
                log_error("Failed to reallocate memory for ID to slot mapping");
                ent->_inUse = 0; // Mark entity as not in use since we failed to map its ID
                return NULL;
            }

            memcpy(newIdToSlot, manager->idToSlot, sizeof(uint64_t) * manager->maxIdSlots);
            memset(newIdToSlot + manager->maxIdSlots, 0, sizeof(uint64_t) * (newMaxIdSlots - manager->maxIdSlots));
            free(manager->idToSlot);
            manager->idToSlot = newIdToSlot;
        }

        manager->idToSlot[ent->id] = i; // Map ID to slot index

        return ent;
    }

    return NULL;
}

entity_t *entity_new_animated(const entity_manager_t *manager, const int64_t id) {
    entity_t *ent = entity_new(manager, id);
    if (!ent) return NULL;

    ent->flags |= ENT_FLAG_ANIMATED;
    ent->draw = entity_draw_animated;
    ent->update = entity_update_animated;

    return ent;
}

void entity_free(const entity_manager_t *entityManager, entity_t* ent) {
    uint64_t generation;
    if (!ent) return;

     // Clear ID to slot mapping if applicable

    if (ent->flags & ENT_FLAG_ANIMATED) {
        // Assuming model is of type AnimatedSprite when ENT_FLAG_ANIMATED is set
        AnimatedSprite *animatedSprite = (AnimatedSprite *)ent->model;
        if (animatedSprite) {
            animation_sprite_freeSprite(animatedSprite);
        }
    } else {
        if (ent->model) gf2d_sprite_free(ent->model);
    }

    if (ent->destroy) ent->destroy(entityManager, ent);
    if (ent->data) {
        free(ent->data);
        ent->data = NULL;
    }

    memset(ent, 0, sizeof(entity_t));
}

void entity_think_all(const entity_manager_t *manager) {
    size_t i;
    entity_t *ent;
    for (i = 0; i < manager->maxEnts; i++) {
        ent = &manager->ents[i];
        if (ent->_inUse == 0 || !ent->think) continue;
        ent->think(manager, ent);
    }
}
void entity_update_all(const entity_manager_t *manager, const float deltaTime) {
    size_t i;
    entity_t *ent;
    for (i = 0; i < manager->maxEnts; i++) {
        ent = &manager->ents[i];
        if (ent->_inUse == 0 || !ent->update) continue;
        ent->update(manager, ent, deltaTime);
    }
}

void entity_update_animated(const entity_manager_t *entityManager, entity_t *ent, float deltaTime) {
    if (!ent) return;
    if (!(ent->flags & ENT_FLAG_ANIMATED)) return;

    // Assuming model is of type AnimatedSprite when ENT_FLAG_ANIMATED is set
    AnimatedSprite *animatedSprite = (AnimatedSprite *)ent->model;
    if (!animatedSprite) return;

    animation_state_update(animatedSprite->state, deltaTime);
}

void entity_draw(const entity_manager_t *entityManager, entity_t *ent) {
    GFC_Vector2D position;
    if (!ent) return;

    gfc_vector2d_sub(position, ent->position, g_camera.position);
    gf2d_sprite_draw_image(ent->model, position);
}

int64_t entity_next_id(entity_manager_t *entityManager) {
    if (g_game.role == GAME_ROLE_CLIENT) {
        return -1; // Use negative IDs for client-side entities
    } else {
        return entityManager->nextId++; // Use positive IDs for server-side entities
    }
}

void entity_set_id(entity_manager_t *entityManager, entity_t *ent, int64_t id) {
    uint32_t i;
    if (!ent) return;
    ent->id = id;

    if (id < 0) {
        return; // Negative IDs are not mapped, client side
    }

    for (i = 0; i < entityManager->maxEnts; i++) {
        if (&entityManager->ents[i] == ent) {
            if (id >= entityManager->maxIdSlots) {
                uint64_t newMaxIdSlots = entityManager->maxIdSlots * 2;
                uint64_t *newIdToSlot = malloc(sizeof(uint64_t) * newMaxIdSlots);
                if (!newIdToSlot) {
                    log_error("Failed to reallocate memory for ID to slot mapping");
                    return;
                }

                memcpy(newIdToSlot, entityManager->idToSlot, sizeof(uint64_t) * entityManager->maxIdSlots);
                memset(newIdToSlot + entityManager->maxIdSlots, 0, sizeof(uint64_t) * (newMaxIdSlots - entityManager->maxIdSlots));
                free(entityManager->idToSlot);
                entityManager->idToSlot = newIdToSlot;
            }

            entityManager->idToSlot[id] = i; // Map ID to slot index
            return;
        }
    }
}

entity_t * entity_get(const entity_manager_t *manager, int64_t id) {
    if (!manager->idToSlot || id >= manager->maxIdSlots) {
        return NULL;
    }

    uint64_t slotIndex = manager->idToSlot[id];
    if (slotIndex >= manager->maxEnts) {
        return NULL;
    }

    entity_t *ent = &manager->ents[slotIndex];
    if (ent->_inUse == 0 || ent->id != id) {
        return NULL; // ID does not match, likely a stale reference
    }

    return ent;
}

void entity_draw_animated(const entity_manager_t *entityManager, entity_t *ent) {
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
    uint32_t i;
    entity_t *ent;
    for (i = 0; i < manager->maxEnts; i++) {
        ent = &manager->ents[i];
        if (ent->_inUse == 0 || !ent->draw) continue;
        ent->draw(manager, ent);

        entity_draw_debug(manager, ent);
    }
}

void entity_draw_debug(const entity_manager_t *entityManager, entity_t *ent) {
    if (!ent || !__DEBUG) return;

    GFC_Vector2D position;
    gfc_vector2d_sub(position, ent->position, g_camera.position);
    gf2d_draw_rect(
        gfc_rect(position.x + ent->boundingBox.x, position.y + ent->boundingBox.y, ent->boundingBox.w, ent->boundingBox.h),
        GFC_COLOR_DARKBLUE
    );

    gf2d_draw_circle(position, 3, GFC_COLOR_RED);
}
