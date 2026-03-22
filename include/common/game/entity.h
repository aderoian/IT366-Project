#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>

#include "gfc_shape.h"
#include "gfc_vector.h"
#include "common/physics.h"

#define ENTITY_MAX_ID INT64_MAX

#define ENT_FLAG_ANIMATED    0x0001
#define ENT_FLAG_COLLIDE_SOLID      0x0002
#define ENT_FLAG_ENEMY     0x0004

#define ENT_LAYER_DEFAULT 0x0001
#define ENT_LAYER_PLAYER  0x0002
#define ENT_LAYER_TOWER   0x0004
#define ENT_LAYER_PROJECTILE 0x0008
#define ENT_LAYER_ENEMY   0x0010

typedef struct entity_manager_s entity_manager_t;

typedef struct entity_s {
    uint8_t _inUse;
    int64_t id;
    GFC_Vector2D position;
    GFC_Rect boundingBox;
    uint16_t layers;
    float rotation;
    GFC_Vector2D scale;
    uint32_t flags;
    void *data;

    void *model; // Sprite or AnimatedSprite, depends on animated flag
    void (*think)(const entity_manager_t *entityManager, struct entity_s *ent);
    void (*update)(const entity_manager_t *entityManager, struct entity_s *ent, float deltaTime);
    void (*draw)(const entity_manager_t *entityManager, struct entity_s *ent);
    void (*destroy)(const entity_manager_t *entityManager, struct entity_s *ent);
    uint32_t (*collidesWith)(struct entity_s *ent, struct entity_s *other);
} entity_t;

entity_manager_t *entity_init(uint32_t maxEnts);
void entity_close(const entity_manager_t* manager);

entity_t *entity_new(entity_manager_t* manager, int64_t id);
entity_t *entity_new_animated(const entity_manager_t* manager, int64_t id);
void entity_free(const entity_manager_t *entityManager, entity_t *ent);
void entity_draw(const entity_manager_t *entityManager, entity_t *ent);

int64_t entity_next_id(entity_manager_t *entityManager);
void entity_set_id(entity_manager_t *entityManager, entity_t *ent, int64_t id);
entity_t *entity_get(const entity_manager_t *manager, int64_t id);

void entity_draw_animated(const entity_manager_t *entityManager, entity_t *ent);
void entity_update_animated(const entity_manager_t *entityManager, entity_t *ent, float deltaTime);

void entity_think_all(const entity_manager_t *manager);
void entity_update_all(const entity_manager_t *manager, float deltaTime);
void entity_draw_all(const entity_manager_t *manager);

void entity_draw_debug(const entity_manager_t *entityManager, entity_t *ent);

#endif /* ENTITY_H */