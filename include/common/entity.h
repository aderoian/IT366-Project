#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>

#include "gfc_vector.h"
#include "common/physics.h"

#define ENT_FLAG_ANIMATED    0x0001

typedef struct Entity_S {
    uint8_t _inUse;
    GFC_Vector2D position;
    GFC_Vector2D velocity;
    GFC_Vector2D forces;
    phys_AABBShape localBounds; // min and max
    phys_AABBShape worldBounds;
    phys_CollisionHandle sapIndex;
    uint16_t layers;
    float rotation;
    float mass;
    float invMass;
    GFC_Vector2D scale;
    uint32_t flags;

    void *model; // Sprite or AnimatedSprite, depends on animated flag
    void (*think)(struct Entity_S *ent);
    void (*update)(struct Entity_S *ent, float deltaTime);
    void (*draw)(struct Entity_S *ent);
} Entity;

void entity_init(uint32_t maxEnts);
void entity_close(void);

Entity *entity_new(void);
Entity *entity_new_animated(void);
void entity_free(Entity *ent);
void entity_draw(Entity *ent);

void entity_draw_animated(Entity *ent);
void entity_update_animated(Entity *ent, float deltaTime);

void entity_think_all(void);
void entity_update_all(float deltaTime);
void entity_draw_all();

#endif /* ENTITY_H */