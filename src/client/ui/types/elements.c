#include "simple_logger.h"

#include "client/ui/event.h"
#include "client/ui/window.h"
#include "client/ui/types/elements.h"

void container_draw(widget_t *widget) {
    elm_container_data_t *data;
    size_t i;
    if (!widget || !widget->data) {
        return;
    }

    data = (elm_container_data_t *)widget->data;
    if (data->background) {
        gf2d_sprite_draw(data->background, gfc_vector2d(widget->rect.x, widget->rect.y), NULL, NULL, NULL, NULL, NULL, 0);
    }

    for (i = 0; i < data->numElements; i++) {
        if (data->elements[i]) {
            widget_draw(data->elements[i]);
        }
    }
}

void container_destroy(widget_t *widget) {
    elm_container_data_t *data;
    if (!widget || !widget->data) {
        return;
    }

    data = (elm_container_data_t *)widget->data;
    if (data->background) {
        gf2d_sprite_free(data->background);
    }
}

#define ELEMENT_TYPE_STRING(name, cap_name, str) if (strcmp(typeStr, str) == 0) { return ELEMENT_TYPE_##cap_name; }
element_type_t element_type_from_string(const char *typeStr) {
    if (!typeStr) {
        return ELEMENT_TYPE_NONE;
    }

    ELEMENTS(ELEMENT_TYPE_STRING)

    return ELEMENT_TYPE_NONE; // Default to NONE if not found
}
#undef ELEMENT_TYPE_STRING

int elm_container_init(widget_t *widget, const char *spritePath, const size_t maxElements) {
    if (!widget) {
        slog("Failed to create container widget");
        return -1;
    }

    elm_container_data_t *data = gfc_allocate_array(sizeof(elm_container_data_t), 1);
    if (!data) {
        slog("Failed to allocate container data");
        widget_destroy(widget);
        return -1;
    }

    data->background = gf2d_sprite_load_image(spritePath);
    if (!data->background) {
        slog("Failed to load container background sprite");
        free(data);
        widget_destroy(widget);
        return -1;
    }

    data->elements = gfc_allocate_array(sizeof(widget_t*), maxElements);
    if (!data->elements) {
        slog("Failed to allocate container elements array");
        gf2d_sprite_free(data->background);
        free(data);
        widget_destroy(widget);
        return -1;
    }
    data->numElements = 0;

    widget->data = data;
    widget->draw = container_draw;
    widget->destroy = container_destroy;

    return 0;
}

int elm_container_json_load(def_data_t *json, widget_t *widget) {
    widget_t *sibling;
    def_data_t *elements, *element;
    int i, count;
    const char *spritePath;
    elm_container_data_t *data;
    if (!json) {
        slog("Invalid JSON for container");
        return -1;
    }

    elements = def_data_get_array(json, "elements");
    if (!elements) {
        count = 0; // No elements, set count to 0
    } else {
        def_data_array_get_count(elements, &count);
    }
    
    spritePath = def_data_get_string(json, "image");
    if (elm_container_init(widget, spritePath, count) < 0) {
        slog("Failed to initialize container widget");
        widget_destroy(widget);
        return -1;
    }

    data = (elm_container_data_t *)widget->data;
    for (i = 0; i < count; i++) {
        element = def_data_array_get_nth(elements, i);
        if (!def_data_array_get_nth(elements, i)) {
            slog("Failed to get element at index %d", i);
            continue;
        }

        sibling = widget_load_from_json(element, widget);
        if (!sibling) {
            slog("Failed to load element widget from JSON at index %d", i);
            continue;
        }

        data->elements[data->numElements++] = sibling;
    }
    
    
    return 1;
}

void button_draw(widget_t *widget) {
    elm_button_data_t *data;
    if (!widget || !widget->data) {
        return;
    }

    data = (elm_button_data_t *)widget->data;
    if (data->background) {
        gf2d_sprite_draw(data->background, gfc_vector2d(widget->rect.x, widget->rect.y), NULL, NULL, NULL, NULL, NULL, 0);
    }
}

int button_on_event(widget_t *widget, const window_event_t *event) {
    int mouseX, mouseY;
    if (!widget || !widget->data || !event) {
        return 0;
    }

    if (event->type == EVENT_TYPE_MOUSE && event->data.mouse.type == MOUSE_EVENT_BUTTON_DOWN &&
        (event->data.mouse.buttons & EVENT_MOUSE_BUTTON_LEFT)) {
        SDL_GetMouseState(&mouseX, &mouseY);
        if (mouseX >= widget->rect.x && mouseX <= widget->rect.x + widget->rect.w &&
            mouseY >= widget->rect.y && mouseY <= widget->rect.y + widget->rect.h) {
            // Button was clicked
            if (widget->parent && widget->parent->on_button_click) {
                widget->parent->on_button_click((window_t *)widget->parent, widget->id);
                return 1;
            }
        }
    }

    return 0;
}

void button_destroy(widget_t *widget) {
    elm_button_data_t *data;
    if (!widget || !widget->data) {
        return;
    }

    data = (elm_button_data_t *)widget->data;
    if (data->background) {
        gf2d_sprite_free(data->background);
    }
}

int elm_button_init(widget_t *widget, const char *text, const char *spritePath) {
    if (!widget) {
        slog("Failed to create button widget");
        return -1;
    }

    elm_button_data_t *data = gfc_allocate_array(sizeof(elm_button_data_t), 1);
    if (!data) {
        slog("Failed to allocate button data");
        widget_destroy(widget);
        return -1;
    }

    if (text) {
        strncpy(data->text, text, sizeof(data->text) - 1);
        data->text[sizeof(data->text) - 1] = '\0';
    }
    
    data->background = gf2d_sprite_load_image(spritePath);
    if (!data->background) {
        slog("Failed to load button background sprite");
        free(data);
        widget_destroy(widget);
        return -1;
    }

    widget->data = data;
    widget->draw = button_draw;
    widget->handle_event = button_on_event;
    widget->destroy = button_destroy;

    return 0;
}

int elm_button_json_load(def_data_t *json, widget_t *widget) {
    const char *text, *spritePath;
    if (!json) {
        slog("Invalid JSON for container");
        return -1;
    }

    text = def_data_get_string(json, "text");
    spritePath = def_data_get_string(json, "image");

    if (elm_button_init(widget, text, spritePath) < 0) {
        slog("Failed to initialize button widget");
        widget_destroy(widget);
        return -1;
    }
    
    return 0;
}

