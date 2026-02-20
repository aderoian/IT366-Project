#include "simple_logger.h"
#include "gfc_types.h"

#include "common/def.h"

#include "client/ui/window.h"
#include "client/ui/widget.h"

#include "client/ui/types/elements.h"

typedef struct widget_manager_s {
    widget_t *widgets;
    size_t maxWidgets;
} widget_manager_t;

widget_manager_t g_widgetManager;

void widget_init(const size_t maxWidgets) {
    g_widgetManager.widgets = gfc_allocate_array(sizeof(widget_t), maxWidgets);
    g_widgetManager.maxWidgets = maxWidgets;
    atexit(widget_close);
}
void widget_close(void) {
    size_t i;
    if (g_widgetManager.widgets) {
        for (i = 0; i < g_widgetManager.maxWidgets; i++) {
            if (g_widgetManager.widgets[i]._refCount) {
                widget_destroy(&g_widgetManager.widgets[i]);
            }
        }
        free(g_widgetManager.widgets);
        g_widgetManager.widgets = NULL;
    }
}

widget_t *widget_create(const char *id) {
    size_t i, j = g_widgetManager.maxWidgets; // Initialize j to maxWidgets to indicate no free slot found
    widget_t *widget;
    for (i = 0; i < g_widgetManager.maxWidgets; i++) {
        widget = &g_widgetManager.widgets[i];
        if (widget->_refCount && strcmp(widget->id, id) == 0) {
            widget->_refCount++;
            return widget;
        }

        if (!widget->_refCount) {
            j = i;
        }
    }

    if (j < g_widgetManager.maxWidgets) {
        widget = &g_widgetManager.widgets[j];
        strncpy(widget->id, id, sizeof(widget->id) - 1);
        widget->id[sizeof(widget->id) - 1] = '\0';
        widget->visible = 1;
        widget->_refCount = 1;
        return widget;
    }

    slog("Failed to create widget: no free slot found");
    return NULL; // No free slot found
}

void widget_destroy(widget_t *widget) {
    if (!widget) {
        return;
    }

    if (widget->_refCount > 0) {
        widget->_refCount--;
        if (widget->_refCount > 0) {
            return; // Still in use, don't destroy
        }
    }

    if (widget->child) {
        widget_destroy(widget->child);
    }

    if (widget->destroy) {
        widget->destroy(widget);
    }
    if (widget->data) {
        free(widget->data);
    }
    memset(widget, 0, sizeof(widget_t));
}

void widget_set_child(widget_t *parent, widget_t *child) {
    if (!parent) {
        return;
    }
    parent->child = child;
    if (child) {
        child->parent = parent->parent;
    }
}

void widget_set_position(widget_t *widget, const GFC_Vector2D position) {
    if (!widget) {
        return;
    }
    widget->rect.x = position.x;
    widget->rect.y = position.y;
}

void widget_set_offset(widget_t *widget, const GFC_Vector2D offset) {
    if (!widget) {
        return;
    }
    widget->rect.x += offset.x;
    widget->rect.y += offset.y;
}

void widget_set_size(widget_t *widget, const int width, const int height) {
    if (!widget) {
        return;
    }
    widget->rect.w = width;
    widget->rect.h = height;
}

void widget_set_visible(widget_t *widget, const uint8_t visible) {
    if (!widget) {
        return;
    }
    widget->visible = visible;
}

void widget_draw(widget_t *widget) {
    if (!widget || !widget->visible) {
        return;
    }
    if (widget->draw) {
        widget->draw(widget);
    }
    if (widget->child) {
        widget_draw(widget->child);
    }
}

void widget_update(widget_t *widget, const float deltaTime) {
    if (!widget || !widget->visible) {
        return;
    }
    if (widget->update) {
        widget->update(widget, deltaTime);
    }
    if (widget->child) {
        widget_update(widget->child, deltaTime);
    }
}

int widget_handle_event(widget_t *widget, const window_event_t *event) {
    if (!widget || !widget->visible) {
        return 0;
    }
    if (widget->handle_event && widget->handle_event(widget, event)) {
        return 1; // Event handled
    }
    if (widget->child && widget_handle_event(widget->child, event)) {
        return 1; // Event handled by child
    }
    return 0; // Event not handled
}

widget_t *widget_load_from_json(def_data_t *json, widget_t *parent) {
    const char *id;
    widget_t *widget;
    def_data_t *child;
    GFC_Vector2D position, size;
    element_type_t type;
    if (!json) {
        return NULL;
    }

    id = def_data_get_string(json, "id");
    if (!id) {
        slog("Widget JSON missing 'id' field");
        return NULL;
    }

    widget = widget_create(id);

    if (def_data_get_vector2d(json, "position", &position)) {
        widget_set_position(widget, position);
    }

    if (parent) {
        widget_set_offset(widget, gfc_vector2d(parent->rect.x, parent->rect.y));
    }

    if (def_data_get_vector2d(json, "size", &size)) {
        widget_set_size(widget, (int)size.x, (int)size.y);
    }

    type = element_type_from_string(def_data_get_string(json, "type"));
    if (type != ELEMENT_TYPE_NONE && element_json_loaders[type]) {
        if (element_json_loaders[type](json, widget) < 0) {
            slog("Failed to load widget of type %s from JSON", element_type_strings[type]);
            widget_destroy(widget);
            return NULL;
        }
    }

    child = def_data_get_obj(json, "child");
    if (child) {
        widget_t *childWidget = widget_load_from_json(child, widget);
        if (childWidget) {
            widget_set_child(widget, childWidget);
        } else {
            slog("Failed to load child widget from JSON");
        }
    }
    return widget;
}
