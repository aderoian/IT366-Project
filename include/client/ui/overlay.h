#ifndef OVERLAY_H
#define OVERLAY_H

#include <stddef.h>
#include <stdint.h>

#include "gfc_vector.h"

#define OVERLAY_TYPE_SIMPLE 0

#define OVERLAY_ELEMENTS(X) \
X(hudbar, HUDBAR, SIMPLE)

typedef struct overlay_element_s {
    GFC_Vector2D position;
    void *data;
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
} overlay_t;

void overlay_init(overlay_t *overlay, size_t initialCapacity, const char *config);

void overlay_destroy(overlay_t *overlay);

void overlay_add_element(overlay_t *overlay, overlay_element_t *element);

void overlay_remove_element(overlay_t *overlay, overlay_element_t *element);

void overlay_draw(const overlay_t *overlay);

void overlay_update(overlay_t *overlay, float deltaTime);

#define OVERLAY_CREATION(name, cap_name, type) TYPE_##cap_name,
typedef enum overlay_element_type_e {
    OVERLAY_ELEMENTS(OVERLAY_CREATION)
    TYPE_NONE
} overlay_element_type_t;
#undef OVERLAY_CREATION

overlay_element_type_t overlay_element_type_from_string(const char *typeStr);

#define OVERLAY_CREATION(name, cap_name, type) OVERLAY_CREATION_##type(name)
#define OVERLAY_CREATION_SIMPLE(name) \
    void overlay_element_create_##name(overlay_element_t *element, GFC_Vector2D position, const char* sprite);

OVERLAY_ELEMENTS(OVERLAY_CREATION)

#undef OVERLAY_CREATION
#undef OVERLAY_CREATION_SIMPLE

#endif /* OVERLAY_H */