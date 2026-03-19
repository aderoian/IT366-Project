#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>

#include "gfc_vector.h"
#include "common/physics.h"

#define ENT_FLAG_ANIMATED    0x0001

typedef struct entity_manager_s entity_manager_t;

typedef struct entity_s {
    uint8_t _inUse;
    uint64_t id;
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
    void *data;

    void *model; // Sprite or AnimatedSprite, depends on animated flag
    void (*think)(const entity_manager_t *entityManager, struct entity_s *ent);
    void (*update)(const entity_manager_t *entityManager, struct entity_s *ent, float deltaTime);
    void (*draw)(const entity_manager_t *entityManager, struct entity_s *ent);
} entity_t;

entity_manager_t *entity_init(uint32_t maxEnts);
void entity_close(const entity_manager_t* manager);

entity_t *entity_new(const entity_manager_t* manager);
entity_t *entity_new_animated(const entity_manager_t* manager);
void entity_set_id(const entity_manager_t *manager, entity_t *ent, uint64_t id);
void entity_free(const entity_manager_t *entityManager, entity_t *ent);
void entity_draw(const entity_manager_t *entityManager, entity_t *ent);

void entity_draw_animated(const entity_manager_t *entityManager, entity_t *ent);
void entity_update_animated(const entity_manager_t *entityManager, entity_t *ent, float deltaTime);

void entity_think_all(const entity_manager_t *manager);
void entity_update_all(const entity_manager_t *manager, float deltaTime);
void entity_draw_all(const entity_manager_t *manager);

#endif /* ENTITY_H */