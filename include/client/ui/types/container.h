#ifndef WINDOW_CONTAINER_H
#define WINDOW_CONTAINER_H

#include "client/ui/widget.h"
#include "common/render/gf2d_sprite.h"

/**
 * @brief Data structure for a container element, which can hold multiple child elements and has a background sprite.
 */
typedef struct container_data_s {
    Sprite *background;
    widget_t **elements;
    size_t numElements, maxElements;
} container_data_t;

widget_t *container_create(const char *id, GFC_Vector2D position, GFC_Vector2D size, const char *spritePath, size_t maxElements);

void container_add_widget(widget_t* container, widget_t *widget);

#endif /* WINDOW_CONTAINER_H */
