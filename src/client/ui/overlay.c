#include <stdlib.h>
#include <string.h>

#include "client/gf2d_sprite.h"

#include "client/ui/overlay.h"

#include "common/def.h"

#define OVERLAY_TYPE(name, cap_name, type) OVERLAY_TYPE_##type,
static int overlay_categories[] = {
    OVERLAY_ELEMENTS(OVERLAY_TYPE)
};
#undef OVERLAY_TYPE

#define OVERLAY_CREATION(name, cap_name, type) (void*) overlay_element_create_##name
static void* overlay_element_creators[] = {
    OVERLAY_ELEMENTS(OVERLAY_CREATION)
};
#undef OVERLAY_CREATION

void overlay_init(overlay_t *overlay, const size_t initialCapacity, const char *config) {
    def_data_t *def, *elements, *elementDef;
    int i, c;
    const char *sprite;
    overlay_element_type_t type;
    GFC_Vector2D position;
    overlay_element_t element;
    if (!overlay) {
        return;
    }

    overlay->elements = malloc(sizeof(overlay_element_t) * initialCapacity);
    overlay->maxElements = initialCapacity;
    overlay->visible = 1;
    if (!overlay->elements) {
        return;
    }
    memset(overlay->elements, 0, sizeof(overlay_element_t) * initialCapacity);

    def = def_load(config);
    elements = def_data_get_array(def, "elements");
    def_data_array_get_count(elements, &c);
    for (i = 0; i < c; i++) {
        elementDef = def_data_array_get_nth(elements, i);
        type = overlay_element_type_from_string(def_data_get_string(elementDef, "type"));

        if (type != TYPE_NONE && overlay_categories[type] == OVERLAY_TYPE_SIMPLE) {
            void (*creator)(overlay_element_t *, GFC_Vector2D, const char*) = overlay_element_creators[type];
            if (creator) {
                def_data_get_vector2d(elementDef, "position", &position);
                sprite = def_data_get_string(elementDef, "sprite");
                creator(&element, position, sprite);
                overlay_add_element(overlay, &element);
            }
        }
    }
}

void overlay_destroy(overlay_t *overlay) {
    if (!overlay || !overlay->elements) {
        return;
    }
    for (size_t i = 0; i < overlay->maxElements; i++) {
        if (overlay->elements[i]._inuse && overlay->elements[i].destroy) {
            overlay->elements[i].destroy(&overlay->elements[i]);
        }
    }

    free(overlay->elements);
    overlay->elements = NULL;
    overlay->maxElements = 0;
}

void overlay_add_element(overlay_t *overlay, overlay_element_t *element) {
    size_t i;
    if (!overlay || !element) {
        return;
    }

    for (i = 0; i < overlay->maxElements; i++) {
        if (!overlay->elements[i]._inuse) {
            overlay->elements[i] = *element;
            overlay->elements[i]._inuse = 1;
            return;
        }
    }
}

void overlay_remove_element(overlay_t *overlay, overlay_element_t *element) {
    size_t i;
    if (!overlay || !element) {
        return;
    }

    for (i = 0; i < overlay->maxElements; i++) {
        if (&overlay->elements[i] == element) {
            if (overlay->elements[i].destroy) {
                overlay->elements[i].destroy(&overlay->elements[i]);
            }
            overlay->elements[i]._inuse = 0;
            memset(&overlay->elements[i], 0, sizeof(overlay_element_t));
            return;
        }
    }
}

void overlay_draw(const overlay_t *overlay) {
    size_t i;
    if (!overlay || !overlay->visible) {
        return;
    }

    for (i = 0; i < overlay->maxElements; i++) {
        if (overlay->elements[i]._inuse && overlay->elements[i].visible && overlay->elements[i].draw) {
            overlay->elements[i].draw(&overlay->elements[i]);
        }
    }
}

void overlay_update(overlay_t *overlay, const float deltaTime) {
    size_t i;
    if (!overlay || !overlay->visible) {
        return;
    }

    for (i = 0; i < overlay->maxElements; i++) {
        if (overlay->elements[i]._inuse && overlay->elements[i].visible && overlay->elements[i].update) {
            overlay->elements[i].update(&overlay->elements[i], deltaTime);
        }
    }
}

void element_simple_draw(overlay_element_t *element) {
    if (!element || !element->data) {
        return;
    }
    gf2d_sprite_draw(element->data, element->position, NULL, NULL, NULL, NULL, NULL, 0);
}

void element_simple_destroy(overlay_element_t *element) {
    if (!element || !element->data) {
        return;
    }
    gf2d_sprite_free(element->data);
    element->data = NULL;
}

#define OVERLAY_CREATION(name, cap_name, type) \
    if (strcmp(typeStr, #name) == 0) { \
        return TYPE_##cap_name; \
    }
overlay_element_type_t overlay_element_type_from_string(const char *typeStr) {
    if (!typeStr) {
        return TYPE_NONE;
    }

    OVERLAY_ELEMENTS(OVERLAY_CREATION)
    return TYPE_NONE;
}
#undef OVERLAY_CREATION

#define OVERLAY_CREATION(name, cap_name, type) OVERLAY_CREATION_##type(name)
#define OVERLAY_CREATION_SIMPLE(name) \
    void overlay_element_create_##name(overlay_element_t *element, GFC_Vector2D position, const char* sprite) { \
        if (!element) { \
            return; \
        } \
        memset(element, 0, sizeof(overlay_element_t)); \
        element->position = position; \
        element->visible = 1; \
        element->draw = element_simple_draw; \
        element->update = NULL; \
        element->destroy = element_simple_destroy; \
        element->data = gf2d_sprite_load_image(sprite); \
    }

OVERLAY_ELEMENTS(OVERLAY_CREATION)

#undef OVERLAY_CREATION
#undef OVERLAY_CREATION_SIMPLE