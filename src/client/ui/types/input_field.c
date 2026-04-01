#include "client/ui/types/input_field.h"

#include "gfc_input.h"
#include "gfc_string.h"
#include "common/logger.h"
#include "common/render/gf2d_draw.h"
#include "common/render/gf2d_font.h"

typedef struct input_field_data_s {
    GFC_String *text;
    GFC_String *placeholder;
    int fontSize;
    uint8_t focused;
} input_field_data_t;

void input_field_update(struct overlay_element_s *element, float deltaTime);
void input_field_destroy(struct overlay_element_s *element);
void input_field_draw(struct overlay_element_s *element);

overlay_element_t * input_field_create(GFC_Vector2D position, GFC_Vector2D size, const char *placeholder, int fontSize) {
    overlay_element_t *element = gfc_allocate_array(sizeof(overlay_element_t), 1);
    if (!element) {
        log_error("Failed to allocate memory for input field element");
        return NULL;
    }

    input_field_data_t *data = gfc_allocate_array(sizeof(input_field_data_t), 1);
    if (!data) {
        log_error("Failed to allocate memory for input field data");
        free(element);
        return NULL;
    }

    data->text = gfc_string_new();
    data->placeholder = gfc_string(placeholder);
    data->fontSize = fontSize;
    data->focused = 0;

    element->type = TYPE_INPUT;
    element->position = position;
    element->size = size;
    element->bounds = gfc_rect(position.x, position.y, size.x, size.y);
    element->data = 0; // You can use this to store additional data if needed
    element->dataPtr = data;
    element->sprite = NULL; // You can assign a sprite for the input field background if desired
    element->_inuse = 1;
    element->visible = 1;

    element->update = input_field_update;
    element->draw = input_field_draw;
    element->destroy = input_field_destroy;

    return element;
}

const char * input_field_get_text(struct overlay_element_s *element) {
    if (!element) {
        return NULL;
    }

    input_field_data_t *data = (input_field_data_t *)element->dataPtr;
    if (data->text && strlen(data->text->buffer) > 0) {
        return data->text->buffer;
    }

    return NULL;
}

void input_field_update(struct overlay_element_s *element, float deltaTime) {
    int x, y, i, j;
    uint32_t mouseState = SDL_GetMouseState(&x, &y);

    input_field_data_t *data = (input_field_data_t *)element->dataPtr;
    if (data->focused) {
        char c[5] = {0};
        j = 0;
        char test[2] = "\0\0";
        if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            if (!gfc_point_in_rect(gfc_vector2d(x, y), element->bounds)) {
                data->focused = 0; // Set focus to this input field
                return;
            }
        }

        if (gfc_input_key_pressed("BACKSPACE")) {
            if (strlen(data->text->buffer) > 0) {
                data->text->buffer[--data->text->length] = '\0'; // Remove last character
            }
            return;
        }

        if (gfc_input_key_pressed("SPACE")) {
            gfc_string_append(data->text, " ");
            return;
        }

        for (i = 46; i < 123; i++) {
            test[0] = i;
            if (gfc_input_key_pressed(test)) {
                c[j] = i;
                j++;
            }
        }

        gfc_string_append(data->text, c);
    } else {
        // Check if the mouse is over the input field to set focus
        if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            if (gfc_point_in_rect(gfc_vector2d(x, y), element->bounds)) {
                data->focused = 1; // Set focus to this input field
            }
        }
    }
}

void input_field_destroy(struct overlay_element_s *element) {
    input_field_data_t *data = (input_field_data_t *)element->dataPtr;
    if (data) {
        if (data->text) free(data->text);
        if (data->text) free(data->placeholder);
        free(data);
    }
    free(element);
}

void input_field_draw(struct overlay_element_s *element) {
    input_field_data_t *data = (input_field_data_t *)element->dataPtr;
    if (data->focused) {
        // Draw the input field background (you can use a sprite or a simple rectangle)
        gf2d_draw_rect_filled(element->bounds, gfc_color(0.8, 0.8, 0.8, 1)); // Light gray background

        // Draw the text
        if (data->text && strlen(data->text->buffer) > 0) {
            gf2d_font_draw_text(data->fontSize, data->text->buffer, gfc_vector2d(element->position.x + 5, element->position.y + 5));
        } else {
            // Draw placeholder text if no input
            gf2d_font_draw_text(data->fontSize, data->placeholder->buffer, gfc_vector2d(element->position.x + 5, element->position.y + 5));
        }
    } else {
        // Draw the input field background (you can use a sprite or a simple rectangle)
        gf2d_draw_rect_filled(element->bounds, gfc_color(0.9, 0.9, 0.9, 1)); // Lighter gray background

        // Draw the text
        if (data->text && strlen(data->text->buffer) > 0) {
            gf2d_font_draw_text(data->fontSize, data->text->buffer, gfc_vector2d(element->position.x + 5, element->position.y + 5));
        } else {
            // Draw placeholder text if no input
            gf2d_font_draw_text(data->fontSize, data->placeholder->buffer, gfc_vector2d(element->position.x + 5, element->position.y + 5));
        }
    }
}
