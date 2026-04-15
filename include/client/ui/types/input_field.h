#ifndef INPUT_FIELD_H
#define INPUT_FIELD_H

#include "gfc_vector.h"
#include "client/ui/overlay.h"
#include "client/ui/widget.h"
#include "gfc_string.h"

typedef struct input_field_data_s {
    GFC_String *text;
    GFC_String *placeholder;
    int fontSize;
    uint8_t focused;
    Sprite *sprite;
} input_field_data_t;

overlay_element_t *input_field_create(GFC_Vector2D position, GFC_Vector2D size, const char *placeholder, int fontSize);

const char *input_field_get_text(struct overlay_element_s *element);

widget_t *input_create(const char *id, GFC_Vector2D position, GFC_Vector2D size, const char *placeholder, int fontSize);

const char *input_get_text(widget_t *element);

#endif /* INPUT_FIELD_H */