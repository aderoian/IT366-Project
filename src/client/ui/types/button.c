#include "client/ui/types/button.h"

#include "common/logger.h"

typedef struct button_data_s {
    void (*onClick)(overlay_element_t *element);
    float cooldown;
} button_data_t;

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
