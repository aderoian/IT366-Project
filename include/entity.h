#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>

#include "gfc_vector.h"
#include "gf2d_sprite.h"

typedef struct Entity_S {
    uint8_t _inUse;
    GFC_Vector2D position;
    float rotation;
    GFC_Vector2D scale;

    Sprite *model;
    void (*think)(struct Entity_S *ent);
    void (*update)(struct Entity_S *ent);
    void (*draw)(struct Entity_S *ent);
} Entity;

void entity_init(uint32_t maxEnts);
void entity_close();

Entity *entity_new();
void entity_free(Entity *ent);
void entity_draw(Entity *ent);

void entity_think_all();
void entity_update_all();
void entity_draw_all();

#endif /* ENTITY_H */