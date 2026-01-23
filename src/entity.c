#include "entity.h"

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

        return ent;
    }

    return NULL;
}

void entity_free(Entity* ent) {
    if (!ent) return;

    if (ent->model) gf2d_sprite_free(ent->model);
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
void entity_update_all() {
    uint32_t i;
    Entity *ent;
    for (i = 0; i < ent_manager.maxEnts; i++) {
        ent = &ent_manager.ents[i];
        if (ent->_inUse == 0 || !ent->think) continue;
        ent->update(ent);
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