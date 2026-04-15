#include "client/ui/types/container.h"

#include "client/ui/event.h"
#include "common/logger.h"

void container_update(widget_t *widget, float deltaTime) {
    container_data_t *data;
    size_t i;
    if (!widget || !widget->data) {
        return;
    }

    data = (container_data_t *)widget->data;
    for (i = 0; i < data->numElements; i++) {
        if (data->elements[i]) {
            widget_update(data->elements[i], deltaTime);
        }
    }
}

void container_draw(widget_t *widget) {
    container_data_t *data;
    size_t i;
    if (!widget || !widget->data) {
        return;
    }

    data = (container_data_t *)widget->data;
    if (data->background) {
        gf2d_sprite_draw(data->background, gfc_vector2d(widget->rect.x, widget->rect.y), NULL, NULL, NULL, NULL, NULL, 0);
    }

    for (i = 0; i < data->numElements; i++) {
        if (data->elements[i]) {
            widget_draw(data->elements[i]);
        }
    }
}

int container_on_event(widget_t *widget, const window_event_t *event) {
    container_data_t *data;
    size_t i;
    if (!widget || !widget->data || !event) {
        return 0;
    }

    data = (container_data_t *)widget->data;
    for (i = 0; i < data->numElements; i++) {
        if (data->elements[i] && widget_handle_event(data->elements[i], event)) {
            return 1; // Event handled by an element
        }
    }

    return 0;
}

void container_destroy(widget_t *widget) {
    container_data_t *data;
    if (!widget || !widget->data) {
        return;
    }

    data = (container_data_t *)widget->data;
    if (data->background) {
        gf2d_sprite_free(data->background);
    }
}

widget_t *container_create(const char *id, GFC_Vector2D position, GFC_Vector2D size, const char *spritePath, const size_t maxElements) {
    widget_t *widget = widget_create(id);

    if (!widget) {
        log_info("Failed to create container widget");
        return NULL;
    }

    widget->rect = gfc_rect(position.x, position.y, size.x, size.y);

    container_data_t *data = gfc_allocate_array(sizeof(container_data_t), 1);
    if (!data) {
        log_info("Failed to allocate container data");
        widget_destroy(widget);
        return NULL;
    }

    data->background = gf2d_sprite_load_image(spritePath);
    if (!data->background) {
        log_info("Failed to load container background sprite");
        free(data);
        widget_destroy(widget);
        return NULL;
    }

    data->elements = gfc_allocate_array(sizeof(widget_t*), maxElements);
    if (!data->elements) {
        log_info("Failed to allocate container elements array");
        gf2d_sprite_free(data->background);
        free(data);
        widget_destroy(widget);
        return NULL;
    }
    data->numElements = 0;
    data->maxElements = maxElements;

    widget->data = data;
    widget->update = container_update;
    widget->draw = container_draw;
    widget->handle_event = container_on_event;
    widget->destroy = container_destroy;

    return widget;
}

void container_add_widget(widget_t *container, widget_t *widget) {
    container_data_t *data;
    if (!container || !container->data || !widget) {
        return;
    }

    widget->rect.x = widget->rect.x + container->rect.x;
    widget->rect.y = widget->rect.y + container->rect.y;

    widget->parent = container->parent;
    data = (container_data_t *)container->data;
    if (data->numElements < data->maxElements) {
        data->elements[data->numElements++] = widget;
    } else {
        log_info("Container has reached maximum number of elements");
    }
}
