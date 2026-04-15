#include "client/ui/types/button.h"

#include "client/ui/event.h"
#include "client/ui/window.h"
#include "common/logger.h"

typedef struct button_data_s {
    void (*onClick)(overlay_element_t *element);
    float cooldown;
} button_data_t;

typedef struct elm_button_data_s {
    char text[64];
    Sprite *background;
    void (*onClick)(widget_t *element);
} elm_button_data_t;

void overlay_button_update(overlay_element_t *element, float floatDelta);
void overlay_button_destroy(overlay_element_t *element);
void overlay_button_draw(overlay_element_t *element);

overlay_element_t * button_create_image(GFC_Vector2D position, GFC_Vector2D size, const char *image,
                                        void(*onClick)(overlay_element_t *element)) {
    overlay_element_t *element = gfc_allocate_array(sizeof(overlay_element_t), 1);
    if (!element) {
        log_error("Failed to allocate memory for input field element");
        return NULL;
    }

    button_data_t *data = gfc_allocate_array(sizeof(button_data_t), 1);
    if (!data) {
        log_error("Failed to allocate memory for input field data");
        free(element);
        return NULL;
    }

    data->onClick = onClick;

    element->type = TYPE_INPUT;
    element->position = position;
    element->sprite = gf2d_sprite_load_image(image);
    element->size = size;
    element->bounds = gfc_rect(position.x, position.y, size.x, size.y);
    element->data = 0; // You can use this to store additional data if needed
    element->dataPtr = data;
    element->_inuse = 1;
    element->visible = 1;

    element->update = overlay_button_update;
    element->draw = overlay_button_draw;
    element->destroy = overlay_button_destroy;

    return element;
}

int button_on_event(widget_t *widget, const window_event_t *event) {
    int mouseX, mouseY;
    if (!widget || !widget->data || !event) {
        return 0;
    }

    if (event->type == EVENT_TYPE_MOUSE && (event->data.mouse.buttons & EVENT_MOUSE_BUTTON_LEFT)) {
        SDL_GetMouseState(&mouseX, &mouseY);
        if (mouseX >= widget->rect.x && mouseX <= widget->rect.x + widget->rect.w &&
            mouseY >= widget->rect.y && mouseY <= widget->rect.y + widget->rect.h) {
            // Button was clicked
            if (((elm_button_data_t *)widget->data)->onClick) {
                ((elm_button_data_t *)widget->data)->onClick(widget);
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

widget_t *button_create(const char *id, GFC_Vector2D position, GFC_Vector2D size, const char *text, const char *image, void(*onClick)(widget_t *widget)) {
    widget_t *widget = widget_create(id);
    
    if (!widget) {
        log_info("Failed to create button widget");
        widget_destroy(widget);
        return NULL;
    }

    widget->rect = gfc_rect(position.x, position.y, size.x, size.y);

    elm_button_data_t *data = gfc_allocate_array(sizeof(elm_button_data_t), 1);
    if (!data) {
        log_info("Failed to allocate button data");
        widget_destroy(widget);
        return NULL;
    }

    if (text) {
        strncpy(data->text, text, sizeof(data->text) - 1);
        data->text[sizeof(data->text) - 1] = '\0';
    }

    data->onClick = onClick;
    data->background = gf2d_sprite_load_image(image);
    if (!data->background) {
        log_info("Failed to load button background sprite");
        free(data);
        widget_destroy(widget);
        return NULL;
    }

    widget->data = data;
    widget->draw = button_draw;
    widget->handle_event = button_on_event;
    widget->destroy = button_destroy;

    return widget;
}

void overlay_button_update(overlay_element_t *element, float floatDelta) {
    int x, y;
    uint32_t button = SDL_GetMouseState(&x, &y);
    button_data_t *data = (button_data_t *)element->dataPtr;

    data->cooldown -= floatDelta;
    if (data->cooldown <= 0 && gfc_point_in_rect(gfc_vector2d(x, y), element->bounds) && (button & SDL_BUTTON(SDL_BUTTON_LEFT))) {
        data->cooldown = 5.0f;
        if (data->onClick) {
            data->onClick(element);
        }
    }
}

void overlay_button_destroy(overlay_element_t *element) {
    free(element->dataPtr);
}

void overlay_button_draw(overlay_element_t *element) {
    if (element->sprite) {
        gf2d_sprite_draw_image(element->sprite, element->position);
    }
}
