#ifndef OVERLAY_H
#define OVERLAY_H

#include <stddef.h>
#include <stdint.h>

#include "gfc_shape.h"
#include "gfc_vector.h"
#include "common/game/tower.h"

#define OVERLAY_TYPE_SIMPLE 0

#define OVERLAY_ELEMENTS(X) \
X(tower_hudbar, TOWER_HUDBAR, SIMPLE) \
X(tower_hb_tower, TOWER_HB_TOWER, SIMPLE) \
X(cycle_container, CYCLE_CONTAINER, SIMPLE)


#define OVERLAY_CREATION(name, cap_name, type) TYPE_##cap_name,
typedef enum overlay_element_type_e {
    OVERLAY_ELEMENTS(OVERLAY_CREATION)
    TYPE_TOWER_OPTIONS,
    TYPE_NONE
} overlay_element_type_t;
#undef OVERLAY_CREATION

struct def_manager_s;

typedef struct overlay_element_s {
    overlay_element_type_t type;
    GFC_Vector2D position;
    GFC_Vector2D size;
    GFC_Rect bounds;
    int data;
    Sprite *sprite;
    uint8_t _inuse;
    uint8_t visible;

    void (*draw)(struct overlay_element_s *element);
    void (*update)(struct overlay_element_s *element, float deltaTime);
    void (*destroy)(struct overlay_element_s *element);
} overlay_element_t;

typedef struct overlay_s {
    overlay_element_t *elements;
    size_t maxElements;
    uint8_t visible;

    overlay_element_t *hoveredElement;
} overlay_t;

void overlay_init(const struct def_manager_s *defManager, overlay_t *overlay, size_t initialCapacity, const char *config);

void overlay_destroy(overlay_t *overlay);

void overlay_add_element(overlay_t *overlay, overlay_element_t *element);

void overlay_remove_element(overlay_t *overlay, overlay_element_t *element);

void overlay_show(overlay_t *overlay);

void overlay_hide(overlay_t *overlay);

void overlay_draw(const overlay_t *overlay);

void overlay_update(overlay_t *overlay, float deltaTime);

int overlay_on_click(overlay_t *overlay, uint32_t mouseButton, int x, int y);

overlay_element_type_t overlay_element_type_from_string(const char *typeStr);

overlay_element_t *overlay_create_simple_element(overlay_element_type_t type, GFC_Vector2D position, GFC_Vector2D size, int data, const char* sprite);

void overlay_get_tower_upgrade_cost(const tower_def_t *def, int level, char *buffer);

#define OVERLAY_CREATION(name, cap_name, type) OVERLAY_CREATION_##type(name)
#define OVERLAY_CREATION_SIMPLE(name) \
    void overlay_element_create_##name(overlay_element_t *element, GFC_Vector2D position, GFC_Vector2D size, int data, const char* sprite);

OVERLAY_ELEMENTS(OVERLAY_CREATION)

#undef OVERLAY_CREATION
#undef OVERLAY_CREATION_SIMPLE

#endif /* OVERLAY_H */